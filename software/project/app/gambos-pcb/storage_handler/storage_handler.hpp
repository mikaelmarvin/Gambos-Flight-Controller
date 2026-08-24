#ifndef STORAGE_HANDLER_HPP
#define STORAGE_HANDLER_HPP

#include "flash_fs.hpp"
#include "sd_fs.hpp"
#include "settings.hpp"
#include "storage_queue.hpp"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include <cstdint>

enum class StorageFile : uint8_t {
    SETTINGS = 0U,
    LOGS = 1U,
};

enum class StorageOperation : uint8_t {
    WRITE = 0U,
    READ = 1U,
};

enum class StorageState : uint8_t {
    NOT_READY = 0U,
    IDLE = 1U,
    SD_TRANSFER = 2U,
};

class StorageHandler {
  public:
    StorageHandler(At25sf128a &flash, SdCard &sd);

    bool Initialize(void);
    void Start(void);

    bool WriteLogs(const uint8_t *data, uint32_t size);
    bool WriteSettings(const Settings &settings);
    bool ReadSettings(Settings &settings);

  private:
    static void TaskFunction(void *pvParameters);

    void HandleSettingsRead(const StorageQueueItem &item);
    void HandleSettingsWrite(const StorageQueueItem &item,
                             bool &log_file_is_open);
    void HandleLogsWrite(const StorageQueueItem &item,
                         bool &log_file_is_open,
                         uint8_t &log_entries);

    void NotifyReadRequester(TaskHandle_t requester, bool success);

    FlashFileSystem _flash_fs;
    SdCardFileSystem _sd_fs;
    StorageQueue _queue;

    StorageState _storage_state{StorageState::NOT_READY};

    inline static StaticSemaphore_t _read_mutex_state{};
    inline static SemaphoreHandle_t _read_mutex_handle{nullptr};
};

static_assert(sizeof(Settings) <= kMaxStorageItemSize);

#endif /* STORAGE_HANDLER_HPP */
