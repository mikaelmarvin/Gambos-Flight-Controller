/**
 * @file storage_handler.cpp
 * @brief One task for storage — policy over FlashFs / SdFs.
 */

#include "storage_handler.hpp"
#include "log.hpp"
#include "messaging/messaging.hpp"

#include "FreeRTOS.h"
#include "task.h"

#include <cstring>

namespace {

constexpr uint32_t kTaskStackWords = 384U;
constexpr UBaseType_t kTaskPriority =
    static_cast<UBaseType_t>(tskIDLE_PRIORITY + 1U);

} // namespace

StorageHandler::StorageHandler(At25sf128a &flash, SdCard &sd)
    : _flash_fs(flash), _sd_fs(sd) {}

bool StorageHandler::Initialize(void) {
    if (!_queue.Initialize()) {
        return false;
    }

    if (_read_mutex_handle == nullptr) {
        _read_mutex_handle =
            xSemaphoreCreateMutexStatic(&_read_mutex_state);
        if (_read_mutex_handle == nullptr) {
            return false;
        }
    }

    if (!_flash_fs.Initialize()) {
        return false;
    }

    return _sd_fs.Initialize();
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

    while (true) {
        switch (self->_storage_state) {
        case StorageState::NOT_READY: {
            LOG("ERROR: Storage not ready");
            break;
        }
        case StorageState::IDLE: {
            StorageQueueItem item = {};
            if (!self->_queue.Receive(item, portMAX_DELAY)) {
                LOG("ERROR: Failed to receive data from storage "
                    "queue");
                continue;
            }

            // Extract the operation and file from the item.
            StorageOperation item_operation =
                static_cast<StorageOperation>(item.operation);
            StorageFile item_file =
                static_cast<StorageFile>(item.file);

            // Handle the request based on the operation and file.
            switch (item_file) {
            case StorageFile::SETTINGS: {
                (item_operation == StorageOperation::READ)
                    ? self->HandleSettingsRead(item)
                    : self->HandleSettingsWrite(item);
                break;
            }
            case StorageFile::LOGS: {
                self->HandleLogsWrite(item);
                break;
            }
            default:
                LOG("ERROR: Invalid storage destination");
                break;
            }
        } break;
        case StorageState::SD_TRANSFER: {

            break;
        }
        default: {
            LOG("ERROR: Invalid storage state");
            break;
        }
        }
    }
}

void StorageHandler::HandleSettingsRead(
    const StorageQueueItem &item) {

    if (item.read.destination == nullptr) {
        LOG("ERROR: Settings read missing destination");
        NotifyReadRequester(item.read.requester, false);
        return;
    }

    if (_flash_fs.IsSettingsFileOpen()) {
        if (!_flash_fs.CloseSettings()) {
            LOG("ERROR: Failed to close settings file");
            NotifyReadRequester(item.read.requester, false);
            return;
        }
    }

    if (!_flash_fs.OpenSettingsForRead()) {
        LOG("ERROR: Failed to open settings file for read");
        NotifyReadRequester(item.read.requester, false);
        return;
    }

    Settings &destination = *item.read.destination;

    const bool read_ok = _flash_fs.ReadSettings(destination);
    if (!read_ok) {
        LOG("ERROR: Failed to read settings from "
            "storage");
    }
    if (!_flash_fs.CloseSettings()) {
        LOG("ERROR: Failed to close settings file");
    }

    NotifyReadRequester(item.read.requester, read_ok);
}

void StorageHandler::HandleSettingsWrite(
    const StorageQueueItem &item) {
    if (item.write.size != sizeof(Settings)) {
        LOG("ERROR: Invalid settings size");
        return;
    }

    if (_flash_fs.IsLogsFileOpen()) {
        if (!_flash_fs.CloseLogs()) {
            LOG("ERROR: Failed to close logs file");
            return;
        }
    }

    if (!_flash_fs.OpenSettingsForWrite()) {
        LOG("ERROR: Failed to open settings file for write");
        return;
    }

    const Settings &settings =
        *reinterpret_cast<const Settings *>(item.write.data);
    if (!_flash_fs.WriteSettings(settings)) {
        LOG("ERROR: Failed to write settings to storage");
    }

    if (!_flash_fs.CloseSettings()) {
        LOG("ERROR: Failed to close settings file");
    }
}

void StorageHandler::HandleLogsWrite(const StorageQueueItem &item) {
    if (item.operation !=
        static_cast<uint8_t>(StorageOperation::WRITE)) {
        LOG("ERROR: Invalid log storage operation");
        return;
    }

    if (!_flash_fs.OpenLogsForWrite()) {
        LOG("ERROR: Failed to open log file");
        return;
    }

    if (!_flash_fs.WriteLogs(item.write.data, item.write.size)) {
        LOG("ERROR: Failed to write logs to storage");
    }

    _log_entries++;

    if (_log_entries >= kMaxLogEntriesBeforeSync) {
        if (!_flash_fs.SyncLogs()) {
            LOG("ERROR: Failed to sync logs to storage");
            return;
        }
        _log_entries = 0U;
    }
}

bool StorageHandler::WriteLogs(const uint8_t *data, uint32_t size) {
    if (data == nullptr || size == 0U || size > kMaxStorageItemSize) {
        return false;
    }

    StorageQueueItem item = {};
    item.file = static_cast<uint8_t>(StorageFile::LOGS);
    item.operation = static_cast<uint8_t>(StorageOperation::WRITE);
    item.write.size = size;
    memcpy(item.write.data, data, size);
    if (!_queue.Send(item, 0)) {
        LOG("ERROR: Failed to send logs to storage queue");
        return false;
    }

    return true;
}

bool StorageHandler::WriteSettings(const Settings &settings) {
    static_assert(sizeof(Settings) <= kMaxStorageItemSize,
                  "Settings size exceeds storage item size");

    StorageQueueItem item = {};
    item.file = static_cast<uint8_t>(StorageFile::SETTINGS);
    item.operation = static_cast<uint8_t>(StorageOperation::WRITE);
    item.write.size = sizeof(Settings);
    std::memcpy(item.write.data, &settings, sizeof(Settings));
    if (!_queue.Send(item, 0)) {
        LOG("ERROR: Failed to send settings to storage "
            "queue");
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
    item.file = static_cast<uint8_t>(StorageFile::SETTINGS);
    item.operation = static_cast<uint8_t>(StorageOperation::READ);
    item.read.requester = xTaskGetCurrentTaskHandle();
    item.read.destination = &settings;

    if (!_queue.Send(item, portMAX_DELAY)) {
        LOG("ERROR: Failed to send settings read to storage "
            "queue");
        (void)xSemaphoreGive(_read_mutex_handle);
        return false;
    }

    uint32_t status = 0;
    (void)xTaskNotifyStateClear(item.read.requester);
    (void)xTaskNotifyWait(0, UINT32_MAX, &status, portMAX_DELAY);

    (void)xSemaphoreGive(_read_mutex_handle);
    return status != 0U;
}

void StorageHandler::NotifyReadRequester(TaskHandle_t requester,
                                         bool success) {
    if (requester == nullptr) {
        return;
    }

    (void)xTaskNotify(
        requester, success ? 1U : 0U, eSetValueWithOverwrite);
}
