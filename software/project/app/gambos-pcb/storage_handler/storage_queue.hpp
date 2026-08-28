#ifndef STORAGE_QUEUE_HPP
#define STORAGE_QUEUE_HPP

#include "settings.hpp"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <cstdint>

inline constexpr uint32_t kMaxStorageQueueItems = 8U;
inline constexpr uint32_t kMaxStorageItemSize = 64U;

struct StorageQueueItem {
    uint8_t file;
    uint8_t operation;

    union {
        struct {
            uint32_t size;
            uint8_t data[kMaxStorageItemSize];
        } write;
        struct {
            Settings *destination;
            TaskHandle_t requester;
        } read;
    };
};

// FreeRTOS mailbox for storage requests. Owns the static queue
// storage; does not own policy or filesystem work.
class StorageQueue {
  public:
    StorageQueue(void) = default;

    bool Initialize(void);
    bool Send(const StorageQueueItem &item, TickType_t timeout);
    bool Receive(StorageQueueItem &item, TickType_t timeout);

  private:
    StaticQueue_t _state{};
    QueueHandle_t _handle{nullptr};
    uint8_t _storage[kMaxStorageQueueItems *
                     sizeof(StorageQueueItem)] = {};
};

#endif // STORAGE_QUEUE_HPP
