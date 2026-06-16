#ifndef STORAGE_HANDLER_HPP
#define STORAGE_HANDLER_HPP

#include "at25sf128a.hpp"
#include "lfs.h"
#include "settings.hpp"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include <cstdint>

namespace {

inline constexpr uint32_t kMaxStorageQueueItems = 8U;
inline constexpr uint32_t kMaxStorageItemSize = 64U;

} // namespace

enum class StorageFile : uint8_t {
    SETTINGS = 0U,
    LOGS = 1U,
};

enum class StorageOperation : uint8_t {
    WRITE = 0U,
    READ = 1U,
};

struct StorageQueueItem {
    StorageFile file;
    StorageOperation operation;

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

class StorageHandler {
  public:
    StorageHandler() = default;

    bool Initialize(void);
    void Start(void);

    bool WriteLogs(const uint8_t *data, uint32_t size);
    bool WriteSettings(const Settings &settings);
    bool ReadSettings(Settings &settings);

  private:
    static void TaskFunction(void *pvParameters);

    void HandleSettingsRequest(const StorageQueueItem &item,
                               bool &log_file_is_open,
                               uint8_t &log_entries);
    void HandleSettingsRead(const StorageQueueItem &item,
                            bool &log_file_is_open);
    void HandleSettingsWrite(const StorageQueueItem &item,
                             bool &log_file_is_open);
    void HandleLogsWrite(const StorageQueueItem &item,
                         bool &log_file_is_open,
                         uint8_t &log_entries);

    bool CloseLogsFileIfOpen(
        bool &log_file_is_open,
        TaskHandle_t notify_on_failure = nullptr);
    void NotifyReadRequester(TaskHandle_t requester, bool success);

    bool OpenSettingsFile(void);
    bool OpenSettingsFileForRead(void);
    bool OpenLogsFile(void);
    bool CloseSettingsFile(void);
    bool CloseLogsFile(void);
    bool SyncLogsToStorage(void);

    bool WriteSettingsToStorage(const Settings &settings);
    bool ReadSettingsFromStorage(Settings &settings);
    bool WriteLogsToStorage(const uint8_t *data, uint32_t size);

    At25sf128a _device{};
    lfs_t _lfs{};
    lfs_config _lfs_cfg{};

    static constexpr uint32_t kLfsCacheSize =
        At25sf128a::kPageProgramBytes;

    uint8_t _file_buffer[kLfsCacheSize]{};
    lfs_file_config _file_config{};

    lfs_file_t _settings_file{};
    lfs_file_t _log_file{};

    static constexpr uint32_t kLfsLookaheadSize = 32U;
    uint8_t _lfs_read_buffer[kLfsCacheSize]{};
    uint8_t _lfs_prog_buffer[kLfsCacheSize]{};
    uint8_t _lfs_lookahead_buffer[kLfsLookaheadSize]{};

    inline static StaticQueue_t _request_queue_state{};
    inline static QueueHandle_t _request_queue_handle{nullptr};
    inline static uint8_t
        _request_queue_storage[kMaxStorageQueueItems *
                               sizeof(StorageQueueItem)] = {0};

    inline static StaticSemaphore_t _read_mutex_state{};
    inline static SemaphoreHandle_t _read_mutex_handle{nullptr};
};

static_assert(sizeof(Settings) <= kMaxStorageItemSize);

#endif /* STORAGE_HANDLER_HPP */
