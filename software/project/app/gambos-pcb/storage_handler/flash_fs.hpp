#ifndef FLASH_FS_HPP
#define FLASH_FS_HPP

#include "at25sf128a.hpp"
#include "lfs.h"
#include "settings.hpp"

#include <cstdint>

// LittleFS volume on the AT25 SPI NOR. Owns mount state and the
// shared file cache (only one file open at a time).
class FlashFileSystem {
  public:
    explicit FlashFileSystem(At25sf128a &flash);

    bool Initialize(void);

    bool OpenSettingsForWrite(void);
    bool OpenSettingsForRead(void);
    bool CloseSettings(void);

    bool OpenLogsForWrite(void);
    bool OpenLogsForRead(void);
    bool CloseLogs(void);
    bool SyncLogs(void);

    bool WriteSettings(const Settings &settings);
    bool ReadSettings(Settings &settings);
    bool WriteLogs(const uint8_t *data, uint32_t size);
    // Bytes read (0 = EOF), or -1 on error.
    int32_t ReadLogs(uint8_t *data, uint32_t size);

    // After a successful SD export: close, delete /logs/0000.bin.
    // Next OpenLogsForWrite recreates an empty file with the same
    // name.
    bool ResetLogs(void);

  private:
    At25sf128a &_flash;
    bool _settings_file_is_open{false};
    bool _logs_file_is_open{false};

    lfs_t _lfs{};
    lfs_config _lfs_cfg{};

    static constexpr uint32_t kLfsCacheSize =
        At25sf128a::kPageProgramBytes;
    static constexpr uint32_t kLfsLookaheadSize = 32U;

    uint8_t _file_buffer[kLfsCacheSize]{};
    lfs_file_config _file_config{};

    lfs_file_t _settings_file{};
    lfs_file_t _log_file{};

    uint8_t _lfs_read_buffer[kLfsCacheSize]{};
    uint8_t _lfs_prog_buffer[kLfsCacheSize]{};
    uint8_t _lfs_lookahead_buffer[kLfsLookaheadSize]{};
};

#endif // FLASH_FS_HPP
