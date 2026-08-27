#ifndef SD_FS_HPP
#define SD_FS_HPP

#include "sd_card.hpp"

#include "ff.h"

#include <cstdint>

// FatFs volume on the SPI SD card. Mount + log file write for
// flash → SD export (PC-readable). Each export creates a new
// LOGS/NNNN.BIN; existing exports are never deleted or overwritten.
class SdCardFileSystem {
  public:
    explicit SdCardFileSystem(SdCard &sd);

    // Registers the card with FatFs disk_* (does not mount / talk
    // SPI).
    bool Initialize(void);

    // Lazy: f_mount triggers SdCard::Init via disk_initialize.
    bool Mount(void);
    bool Unmount(void);
    bool IsMounted(void) const;
    bool IsLogsFileOpen(void) const;

    // Opens LOGS/(max_existing_index + 1).BIN — never reuses gaps
    // left by deleted files on the PC.
    bool OpenLogsForWrite(void);
    bool CloseLogs(void);
    bool WriteLogs(const uint8_t *data, uint32_t size);

  private:
    bool FindNextLogPath(char *path, uint32_t path_len);

    SdCard &_sd;
    FATFS _fs{};
    FIL _log_file{};
    bool _mounted{false};
    bool _logs_file_is_open{false};
};

#endif // SD_FS_HPP
