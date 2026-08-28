/**
 * @file storage_queue.cpp
 * @brief FreeRTOS queue mailbox for storage requests.
 */

#include "storage_queue.hpp"
#include "log.hpp"

bool StorageQueue::Initialize(void) {
    if (_handle != nullptr) {
        return true;
    }

    _handle = xQueueCreateStatic(kMaxStorageQueueItems,
                                 sizeof(StorageQueueItem),
                                 _storage,
                                 &_state);
    return _handle != nullptr;
}

bool StorageQueue::Send(const StorageQueueItem &item,
                        TickType_t timeout) {
    if (_handle == nullptr) {
        LOG("ERROR: Storage queue not initialized\r\n");
        return false;
    }

    return xQueueSend(_handle, &item, timeout) == pdPASS;
}

bool StorageQueue::Receive(StorageQueueItem &item,
                           TickType_t timeout) {
    if (_handle == nullptr) {
        LOG("ERROR: Storage queue not initialized\r\n");
        return false;
    }

    return xQueueReceive(_handle, &item, timeout) == pdPASS;
}
