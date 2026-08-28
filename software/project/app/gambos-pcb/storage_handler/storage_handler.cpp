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

// SD transfer needs room for FatFs/LittleFS call frames + LOG/printf.
// A 512 B sector buffer is kept static (not on this stack).
constexpr uint32_t kTaskStackWords = 512U;
constexpr UBaseType_t kTaskPriority =
    static_cast<UBaseType_t>(tskIDLE_PRIORITY + 1U);

constexpr TickType_t kIdleDelay = pdMS_TO_TICKS(1000U);
constexpr TickType_t kProcessRequestsDelay = pdMS_TO_TICKS(300U);

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

    if (!_sd_fs.Initialize()) {
        return false;
    }

    _instance = this;

    // Subscribe to button info to trigger storage operations with the
    // user button.
    Messaging::Subscribe<topics::ButtonInfo>(
        &StorageHandler::OnButtonInfo);

    // Subscribe to accel sample to trigger storage operations with
    // the accel sample.
    Messaging::Subscribe<topics::AccelSample>(
        &StorageHandler::OnAccelSample);

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

    while (true) {
        switch (self->_storage_state) {
        case StorageState::IDLE: {
            vTaskDelay(kIdleDelay);
            break;
        }
        case StorageState::PROCESS_REQUESTS: {
            StorageQueueItem item = {};
            // There must be a timeout to make the task recheck its
            // state, a button can trigger a state change
            // asynchronously.
            if (!self->_queue.Receive(item, kProcessRequestsDelay)) {
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
                LOG("ERROR: Invalid storage destination\r\n");
                break;
            }
        } break;
        case StorageState::SD_TRANSFER: {
            LOG("INFO: Starting SD transfer\r\n");
            self->HandleSdTransfer();
            self->_storage_state = StorageState::IDLE;
            break;
        }
        default: {
            LOG("ERROR: Invalid storage state\r\n");
            break;
        }
        }
    }
}

void StorageHandler::HandleSettingsRead(
    const StorageQueueItem &item) {

    if (item.read.destination == nullptr) {
        LOG("ERROR: Settings read missing destination\r\n");
        NotifyReadRequester(item.read.requester, false);
        return;
    }

    if (!_flash_fs.CloseLogs()) {
        LOG("ERROR: Failed to close logs file\r\n");
        NotifyReadRequester(item.read.requester, false);
        return;
    }

    if (!_flash_fs.CloseSettings()) {
        LOG("ERROR: Failed to close settings file\r\n");
        NotifyReadRequester(item.read.requester, false);
        return;
    }

    if (!_flash_fs.OpenSettingsForRead()) {
        LOG("ERROR: Failed to open settings file for read\r\n");
        NotifyReadRequester(item.read.requester, false);
        return;
    }

    Settings &destination = *item.read.destination;

    const bool read_ok = _flash_fs.ReadSettings(destination);
    if (!read_ok) {
        LOG("ERROR: Failed to read settings from "
            "storage\r\n");
    }

    if (!_flash_fs.CloseSettings()) {
        LOG("ERROR: Failed to close settings file\r\n");
    }

    NotifyReadRequester(item.read.requester, read_ok);
}

void StorageHandler::HandleSettingsWrite(
    const StorageQueueItem &item) {
    if (item.write.size != sizeof(Settings)) {
        LOG("ERROR: Invalid settings size\r\n");
        return;
    }

    if (!_flash_fs.CloseLogs()) {
        LOG("ERROR: Failed to close logs file\r\n");
        return;
    }

    if (!_flash_fs.CloseSettings()) {
        LOG("ERROR: Failed to close settings file\r\n");
        return;
    }

    if (!_flash_fs.OpenSettingsForWrite()) {
        LOG("ERROR: Failed to open settings file for write\r\n");
        return;
    }

    const Settings &settings =
        *reinterpret_cast<const Settings *>(item.write.data);
    if (!_flash_fs.WriteSettings(settings)) {
        LOG("ERROR: Failed to write settings to storage\r\n");
    }

    if (!_flash_fs.CloseSettings()) {
        LOG("ERROR: Failed to close settings file\r\n");
    }
}

void StorageHandler::HandleLogsWrite(const StorageQueueItem &item) {
    if (item.operation !=
        static_cast<uint8_t>(StorageOperation::WRITE)) {
        LOG("ERROR: Invalid log storage operation\r\n");
        return;
    }

    if (!_flash_fs.OpenLogsForWrite()) {
        LOG("ERROR: Failed to open log file\r\n");
        return;
    }

    if (!_flash_fs.WriteLogs(item.write.data, item.write.size)) {
        LOG("ERROR: Failed to write logs to storage\r\n");
    }

    _log_entries++;

    if (_log_entries >= kMaxLogEntriesBeforeSync) {
        if (!_flash_fs.SyncLogs()) {
            LOG("ERROR: Failed to sync logs to storage\r\n");
            return;
        }
        _log_entries = 0U;
    }
}

void StorageHandler::HandleSdTransfer(void) {

    if (!_flash_fs.CloseLogs()) {
        LOG("ERROR: Failed to close logs file\r\n");
        return;
    }

    if (!_flash_fs.CloseSettings()) {
        LOG("ERROR: Failed to close settings file\r\n");
        return;
    }

    if (!_sd_fs.Mount()) {
        LOG("ERROR: Failed to mount SD card\r\n");
        return;
    }

    if (!_sd_fs.OpenLogsForWrite()) {
        LOG("ERROR: Failed to open logs file for write\r\n");
        _sd_fs.Unmount();
        return;
    }

    if (!_flash_fs.OpenLogsForRead()) {
        LOG("ERROR: Failed to open logs file for read\r\n");
        _sd_fs.CloseLogs();
        _sd_fs.Unmount();
        return;
    }

    // Read 512 bytes at a time (static: keep it off the task stack).
    static uint8_t data[512] = {};
    uint32_t bytes_transferred = 0U;
    bool transfer_ok = true;
    while (true) {
        int32_t bytes_read = _flash_fs.ReadLogs(data, sizeof(data));
        if (bytes_read == 0) {
            LOG("INFO: End of logs file\r\n");
            break;
        } else if (bytes_read < 0) {
            LOG("ERROR: Failed to read logs from storage\r\n");
            transfer_ok = false;
            break;
        }

        if (!_sd_fs.WriteLogs(data, bytes_read)) {
            LOG("ERROR: Failed to write logs to SD card\r\n");
            transfer_ok = false;
            break;
        }

        bytes_transferred += bytes_read;
    }

    if (!transfer_ok) {
        LOG("ERROR: Failed to transfer logs to SD card\r\n");
    } else {
        LOG("INFO: Transferred %u bytes to SD card\r\n",
            static_cast<unsigned>(bytes_transferred));
        _flash_fs.ResetLogs();
    }

    _sd_fs.CloseLogs();
    _sd_fs.Unmount();
}

bool StorageHandler::WriteLogsToFlash(const uint8_t *data,
                                      uint32_t size) {
    if (data == nullptr || size == 0U || size > kMaxStorageItemSize) {
        return false;
    }

    if (_storage_state != StorageState::PROCESS_REQUESTS) {
        return false;
    }

    StorageQueueItem item = {};
    item.file = static_cast<uint8_t>(StorageFile::LOGS);
    item.operation = static_cast<uint8_t>(StorageOperation::WRITE);
    item.write.size = size;
    memcpy(item.write.data, data, size);
    if (!_queue.Send(item, 0)) {
        return false;
    }

    return true;
}

bool StorageHandler::WriteSettingsToFlash(const Settings &settings) {
    static_assert(sizeof(Settings) <= kMaxStorageItemSize,
                  "Settings size exceeds storage item size");

    StorageQueueItem item = {};
    item.file = static_cast<uint8_t>(StorageFile::SETTINGS);
    item.operation = static_cast<uint8_t>(StorageOperation::WRITE);
    item.write.size = sizeof(Settings);
    std::memcpy(item.write.data, &settings, sizeof(Settings));
    if (!_queue.Send(item, 0)) {
        LOG("ERROR: Failed to send settings to storage "
            "queue\r\n");
        return false;
    }

    return true;
}

bool StorageHandler::ReadSettingsFromFlash(Settings &settings) {
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
            "queue\r\n");
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

void StorageHandler::OnButtonInfo(const topics::ButtonInfo &topic) {
    if (_instance == nullptr) {
        return;
    }

    if (topic.button_state != 1U)
        return;

    if (_instance->_storage_state == StorageState::IDLE) {
        _instance->_storage_state = StorageState::PROCESS_REQUESTS;
    } else if (_instance->_storage_state ==
               StorageState::PROCESS_REQUESTS) {
        _instance->_storage_state = StorageState::SD_TRANSFER;
    }

    LOG("INFO: Storage state changed to %u\r\n",
        static_cast<unsigned>(_instance->_storage_state));
}

void StorageHandler::OnAccelSample(const topics::AccelSample &topic) {
    if (_instance == nullptr) {
        return;
    }

    char data_string[32] = {};
    snprintf(data_string,
             sizeof(data_string),
             "x=%5d, y=%5d, z=%5d\r\n",
             topic.x,
             topic.y,
             topic.z);

    _instance->WriteLogsToFlash(
        reinterpret_cast<const uint8_t *>(data_string),
        strlen(data_string));
}