/**
 * @file storage_handler.cpp
 * @brief One task for storage — read and write to storage.
 */

#include "storage_handler.hpp"
#include "log.hpp"
#include "main.h"
#include "messaging/messaging.hpp"
#include "spi.h"

#include "FreeRTOS.h"
#include "task.h"
#include <cstring>

namespace {

constexpr uint32_t kTaskStackWords = 384U;
constexpr UBaseType_t kTaskPriority =
    static_cast<UBaseType_t>(tskIDLE_PRIORITY + 1U);

// LittleFS volume spans the full chip; shrink block_count to reserve
// flash.
constexpr lfs_size_t kLfsBlockCount = static_cast<lfs_size_t>(
    At25sf128a::kCapacityBytes / At25sf128a::kSectorSizeBytes);

constexpr char kSettingsFilePath[] = "/settings.bin";
constexpr char kLogFilePath[] = "/logs/0000.bin";

static int lfs_bd_read(const lfs_config *c,
                       lfs_block_t block,
                       lfs_off_t off,
                       void *buffer,
                       lfs_size_t size) {
    At25sf128a *flash = static_cast<At25sf128a *>(c->context);
    uint32_t addr = block * c->block_size + off;
    return flash->Read(addr, static_cast<uint8_t *>(buffer), size)
               ? LFS_ERR_OK
               : LFS_ERR_IO;
}

static int lfs_bd_prog(const lfs_config *c,
                       lfs_block_t block,
                       lfs_off_t off,
                       const void *buffer,
                       lfs_size_t size) {
    At25sf128a *flash = static_cast<At25sf128a *>(c->context);
    uint32_t addr = block * c->block_size + off;
    return flash->Write(
               addr, static_cast<const uint8_t *>(buffer), size)
               ? LFS_ERR_OK
               : LFS_ERR_IO;
}

static int lfs_bd_erase(const lfs_config *c, lfs_block_t block) {
    At25sf128a *flash = static_cast<At25sf128a *>(c->context);
    uint32_t addr = block * c->block_size;
    return flash->Erase(addr) ? LFS_ERR_OK : LFS_ERR_IO;
}

static int lfs_bd_sync(const lfs_config *c) {
    At25sf128a *flash = static_cast<At25sf128a *>(c->context);
    return flash->EnsureIdle() ? LFS_ERR_OK : LFS_ERR_IO;
}

} // namespace

bool StorageHandler::Initialize(void) {
    if (!_device.Init(&hspi1, FLASH_CS_GPIO_Port, FLASH_CS_Pin)) {
        LOG("ERROR: Failed to initialize external flash");
        return false;
    }

    if (_request_queue_handle == nullptr) {
        _request_queue_handle =
            xQueueCreateStatic(kMaxStorageQueueItems,
                               sizeof(StorageQueueItem),
                               _request_queue_storage,
                               &_request_queue_state);
        if (_request_queue_handle == nullptr) {
            return false;
        }
    }

    if (_read_mutex_handle == nullptr) {
        _read_mutex_handle =
            xSemaphoreCreateMutexStatic(&_read_mutex_state);
        if (_read_mutex_handle == nullptr) {
            return false;
        }
    }

    // Initialize file buffer, the file buffer is a statically
    // allocated buffer that is used to cache data to RAM when opening
    // files.
    std::memset(_file_buffer, 0, sizeof(_file_buffer));
    std::memset(&_file_config, 0, sizeof(_file_config));
    _file_config.buffer = _file_buffer;

    std::memset(&_lfs_cfg, 0, sizeof(_lfs_cfg));
    _lfs_cfg.context = &_device;
    _lfs_cfg.read = lfs_bd_read;
    _lfs_cfg.prog = lfs_bd_prog;
    _lfs_cfg.erase = lfs_bd_erase;
    _lfs_cfg.sync = lfs_bd_sync;
    _lfs_cfg.read_size = At25sf128a::kPageProgramBytes;
    _lfs_cfg.prog_size = At25sf128a::kPageProgramBytes;
    _lfs_cfg.block_size = At25sf128a::kSectorSizeBytes;
    _lfs_cfg.block_count = kLfsBlockCount;
    _lfs_cfg.cache_size = StorageHandler::kLfsCacheSize;
    _lfs_cfg.lookahead_size = StorageHandler::kLfsLookaheadSize;
    _lfs_cfg.block_cycles = 500;
    _lfs_cfg.read_buffer = _lfs_read_buffer;
    _lfs_cfg.prog_buffer = _lfs_prog_buffer;
    _lfs_cfg.lookahead_buffer = _lfs_lookahead_buffer;

    int err = lfs_mount(&_lfs, &_lfs_cfg);
    if (err != LFS_ERR_OK) {
        LOG("ERROR: Failed to mount filesystem: %d", err);
        if (lfs_format(&_lfs, &_lfs_cfg) != LFS_ERR_OK) {
            return false;
        }
        err = lfs_mount(&_lfs, &_lfs_cfg);
        if (err != LFS_ERR_OK) {
            return false;
        }

        LOG("INFO: Formatted filesystem");
    }

    (void)lfs_mkdir(&_lfs, "/logs");

    return true;
}

void StorageHandler::Start(void) {
    configASSERT(xTaskCreate(&StorageHandler::TaskFunction,
                             "storage_handler",
                             kTaskStackWords,
                             this,
                             kTaskPriority,
                             nullptr) == pdPASS);
}

void StorageHandler::TaskFunction(void *pvParameters) {
    StorageHandler *const self =
        static_cast<StorageHandler *>(pvParameters);

    uint8_t log_entries = 0;
    bool log_file_is_open = false;

    while (true) {
        StorageQueueItem item = {};
        if (xQueueReceive(_request_queue_handle,
                          &item,
                          portMAX_DELAY) != pdPASS) {
            LOG("ERROR: Failed to receive data from storage queue");
            continue;
        }

        switch (item.file) {
        case StorageFile::SETTINGS:
            self->HandleSettingsRequest(
                item, log_file_is_open, log_entries);
            break;
        case StorageFile::LOGS:
            self->HandleLogsWrite(
                item, log_file_is_open, log_entries);
            break;
        default:
            LOG("ERROR: Invalid storage destination");
            break;
        }
    }
}

void StorageHandler::HandleSettingsRequest(
    const StorageQueueItem &item,
    bool &log_file_is_open,
    uint8_t &log_entries) {
    log_entries = 0;

    if (item.operation == StorageOperation::READ) {
        HandleSettingsRead(item, log_file_is_open);
        return;
    }

    if (item.operation == StorageOperation::WRITE) {
        HandleSettingsWrite(item, log_file_is_open);
        return;
    }

    LOG("ERROR: Invalid settings storage operation");
}

void StorageHandler::HandleSettingsRead(const StorageQueueItem &item,
                                        bool &log_file_is_open) {
    bool read_ok = false;

    if (item.read.destination == nullptr) {
        LOG("ERROR: Settings read missing destination");
        NotifyReadRequester(item.read.requester, false);
        return;
    }

    if (!CloseLogsFileIfOpen(log_file_is_open, item.read.requester)) {
        return;
    }

    Settings &destination = *item.read.destination;

    if (!OpenSettingsFileForRead()) {
        LOG("ERROR: Failed to open settings file for read");
    } else {
        read_ok = ReadSettingsFromStorage(destination);
        if (!read_ok) {
            LOG("ERROR: Failed to read settings from storage");
        }
        if (!CloseSettingsFile()) {
            LOG("ERROR: Failed to close settings file");
        }
    }

    NotifyReadRequester(item.read.requester, read_ok);
}

void StorageHandler::HandleSettingsWrite(const StorageQueueItem &item,
                                         bool &log_file_is_open) {
    if (!CloseLogsFileIfOpen(log_file_is_open)) {
        return;
    }

    if (!OpenSettingsFile()) {
        LOG("ERROR: Failed to open settings file");
        return;
    }

    if (item.write.size != sizeof(Settings)) {
        LOG("ERROR: Invalid settings size");
        (void)CloseSettingsFile();
        return;
    }

    const Settings &settings =
        *reinterpret_cast<const Settings *>(item.write.data);
    if (!WriteSettingsToStorage(settings)) {
        LOG("ERROR: Failed to write settings to storage");
        (void)CloseSettingsFile();
        return;
    }

    if (!CloseSettingsFile()) {
        LOG("ERROR: Failed to close settings file");
    }
}

void StorageHandler::HandleLogsWrite(const StorageQueueItem &item,
                                     bool &log_file_is_open,
                                     uint8_t &log_entries) {
    if (item.operation != StorageOperation::WRITE) {
        LOG("ERROR: Invalid log storage operation");
        return;
    }

    if (!log_file_is_open) {
        if (!OpenLogsFile()) {
            LOG("ERROR: Failed to open log file");
            return;
        }
        log_file_is_open = true;
    }

    if (!WriteLogsToStorage(item.write.data, item.write.size)) {
        LOG("ERROR: Failed to write logs to storage");
        (void)SyncLogsToStorage();
        return;
    }

    log_entries++;

    if (log_entries >= 10U) {
        if (!SyncLogsToStorage()) {
            LOG("ERROR: Failed to sync logs to storage");
            return;
        }
        log_entries = 0;
    }
}

bool StorageHandler::WriteLogs(const uint8_t *data, uint32_t size) {
    if (data == nullptr || size == 0U || size > kMaxStorageItemSize) {
        return false;
    }

    StorageQueueItem item = {};
    item.file = StorageFile::LOGS;
    item.operation = StorageOperation::WRITE;
    item.write.size = size;
    memcpy(item.write.data, data, size);
    if (xQueueSend(_request_queue_handle, &item, 0) != pdPASS) {
        LOG("ERROR: Failed to send logs to storage queue");
        return false;
    }

    return true;
}

bool StorageHandler::WriteSettings(const Settings &settings) {
    static_assert(sizeof(Settings) <= kMaxStorageItemSize,
                  "Settings size exceeds storage item size");

    StorageQueueItem item = {};
    item.file = StorageFile::SETTINGS;
    item.operation = StorageOperation::WRITE;
    item.write.size = sizeof(Settings);
    std::memcpy(item.write.data, &settings, sizeof(Settings));
    if (xQueueSend(_request_queue_handle, &item, 0) != pdPASS) {
        LOG("ERROR: Failed to send settings to storage queue");
        return false;
    }

    return true;
}

bool StorageHandler::ReadSettings(Settings &settings) {
    if (_read_mutex_handle == nullptr) {
        return false;
    }

    if (xSemaphoreTake(_read_mutex_handle, portMAX_DELAY) != pdTRUE) {
        return false;
    }

    StorageQueueItem item = {};
    item.file = StorageFile::SETTINGS;
    item.operation = StorageOperation::READ;
    item.read.requester = xTaskGetCurrentTaskHandle();
    item.read.destination = &settings;

    if (xQueueSend(_request_queue_handle, &item, portMAX_DELAY) !=
        pdPASS) {
        LOG("ERROR: Failed to send settings read to storage queue");
        (void)xSemaphoreGive(_read_mutex_handle);
        return false;
    }

    uint32_t status = 0;
    (void)xTaskNotifyStateClear(item.read.requester);
    (void)xTaskNotifyWait(0, UINT32_MAX, &status, portMAX_DELAY);

    (void)xSemaphoreGive(_read_mutex_handle);
    return status != 0U;
}

bool StorageHandler::CloseLogsFileIfOpen(
    bool &log_file_is_open, TaskHandle_t notify_on_failure) {
    if (!log_file_is_open) {
        return true;
    }

    if (!CloseLogsFile()) {
        LOG("ERROR: Failed to close log file");
        NotifyReadRequester(notify_on_failure, false);
        return false;
    }

    log_file_is_open = false;
    return true;
}

void StorageHandler::NotifyReadRequester(TaskHandle_t requester,
                                         bool success) {
    if (requester == nullptr) {
        return;
    }

    (void)xTaskNotify(
        requester, success ? 1U : 0U, eSetValueWithOverwrite);
}

bool StorageHandler::CloseSettingsFile(void) {
    return lfs_file_close(&_lfs, &_settings_file) == LFS_ERR_OK;
}

bool StorageHandler::CloseLogsFile(void) {
    return lfs_file_close(&_lfs, &_log_file) == LFS_ERR_OK;
}

bool StorageHandler::OpenSettingsFile(void) {
    return lfs_file_opencfg(&_lfs,
                            &_settings_file,
                            kSettingsFilePath,
                            LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC,
                            &_file_config) == LFS_ERR_OK;
}

bool StorageHandler::OpenSettingsFileForRead(void) {
    return lfs_file_opencfg(&_lfs,
                            &_settings_file,
                            kSettingsFilePath,
                            LFS_O_RDONLY,
                            &_file_config) == LFS_ERR_OK;
}

bool StorageHandler::OpenLogsFile(void) {
    return lfs_file_opencfg(&_lfs,
                            &_log_file,
                            kLogFilePath,
                            LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND,
                            &_file_config) == LFS_ERR_OK;
}

bool StorageHandler::SyncLogsToStorage(void) {
    return lfs_file_sync(&_lfs, &_log_file) == LFS_ERR_OK;
}

bool StorageHandler::WriteSettingsToStorage(
    const Settings &settings) {
    const lfs_ssize_t written = lfs_file_write(
        &_lfs, &_settings_file, &settings, sizeof(settings));
    return written == static_cast<lfs_ssize_t>(sizeof(settings));
}

bool StorageHandler::ReadSettingsFromStorage(Settings &settings) {
    const lfs_ssize_t bytes_read = lfs_file_read(
        &_lfs, &_settings_file, &settings, sizeof(settings));
    return bytes_read == static_cast<lfs_ssize_t>(sizeof(settings));
}

bool StorageHandler::WriteLogsToStorage(const uint8_t *data,
                                        uint32_t size) {
    if (data == nullptr || size == 0U) {
        return false;
    }

    const lfs_ssize_t written =
        lfs_file_write(&_lfs, &_log_file, data, size);
    return written == static_cast<lfs_ssize_t>(size);
}
