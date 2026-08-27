/**
 * @file sd_fs.cpp
 * @brief FatFs on SdCard — disk_* glue, mount, log export file.
 */

#include "sd_fs.hpp"

#include "diskio.h"
#include "log.hpp"

#include <cstdio>
#include <cstring>

namespace {

constexpr char kSdVolumePath[] = "0:";
constexpr char kLogDir[] = "LOGS";
// 8.3 names: LOGS/0000.BIN … LOGS/9999.BIN (never overwrite).
constexpr uint16_t kMaxLogFileIndex = 10000U;
constexpr uint32_t kLogPathMaxLen = 16U; // "LOGS/0000.BIN\0"

SdCard *g_fatfs_sd_card = nullptr;
bool g_fatfs_initialized = false;

void RegisterSdCardForFatFs(SdCard &sd_card) {
    g_fatfs_sd_card = &sd_card;
    g_fatfs_initialized = false;
}

} // namespace

SdCardFileSystem::SdCardFileSystem(SdCard &sd) : _sd(sd) {}

bool SdCardFileSystem::Initialize(void) {
    RegisterSdCardForFatFs(_sd);
    return true;
}

bool SdCardFileSystem::Mount(void) {
    if (_mounted) {
        return true;
    }

    RegisterSdCardForFatFs(_sd);

    const FRESULT fr = f_mount(&_fs, kSdVolumePath, 1);
    if (fr != FR_OK) {
        LOG("ERROR: SD f_mount failed: %d", static_cast<int>(fr));
        _mounted = false;
        return false;
    }

    _mounted = true;
    return true;
}

bool SdCardFileSystem::Unmount(void) {
    if (!_mounted) {
        return true;
    }

    if (!CloseLogs()) {
        return false;
    }

    const FRESULT fr = f_unmount(kSdVolumePath);
    if (fr != FR_OK) {
        LOG("ERROR: SD f_unmount failed: %d", static_cast<int>(fr));
        return false;
    }

    _mounted = false;
    g_fatfs_initialized = false;
    return true;
}

bool SdCardFileSystem::IsMounted(void) const { return _mounted; }

bool SdCardFileSystem::IsLogsFileOpen(void) const {
    return _logs_file_is_open;
}

bool SdCardFileSystem::OpenLogsForWrite(void) {
    if (_logs_file_is_open) {
        return true;
    }

    if (!Mount()) {
        return false;
    }

    FRESULT fr = f_mkdir(kLogDir);
    if ((fr != FR_OK) && (fr != FR_EXIST)) {
        LOG("ERROR: SD f_mkdir failed: %d", static_cast<int>(fr));
        return false;
    }

    // FatFs is configured with FF_USE_LFN=0 (8.3 names only), so
    // paths are short like "LOGS/0000.BIN". kLogPathMaxLen includes
    // room for that path plus the null terminator.
    char path[kLogPathMaxLen] = {};
    if (!FindNextLogPath(path, sizeof(path))) {
        return false;
    }

    // CREATE_NEW: never overwrite an existing export.
    fr = f_open(&_log_file, path, FA_WRITE | FA_CREATE_NEW);
    if (fr != FR_OK) {
        LOG("ERROR: SD f_open logs failed: %d path=%s",
            static_cast<int>(fr),
            path);
        return false;
    }

    LOG("INFO: SD export file %s\r\n", path);
    _logs_file_is_open = true;
    return true;
}

bool SdCardFileSystem::CloseLogs(void) {
    if (!_logs_file_is_open) {
        return true;
    }

    const FRESULT fr = f_close(&_log_file);
    if (fr != FR_OK) {
        LOG("ERROR: SD f_close logs failed: %d",
            static_cast<int>(fr));
        return false;
    }

    _logs_file_is_open = false;
    return true;
}

bool SdCardFileSystem::WriteLogs(const uint8_t *data, uint32_t size) {
    if (!_logs_file_is_open || (data == nullptr) || (size == 0U)) {
        return false;
    }

    UINT written = 0U;
    const FRESULT fr =
        f_write(&_log_file, data, static_cast<UINT>(size), &written);
    if ((fr != FR_OK) || (written != static_cast<UINT>(size))) {
        LOG("ERROR: SD f_write logs failed: %d written=%u",
            static_cast<int>(fr),
            static_cast<unsigned>(written));
        return false;
    }

    return true;
}

bool SdCardFileSystem::FindNextLogPath(char *path,
                                       uint32_t path_len) {
    if ((path == nullptr) || (path_len < kLogPathMaxLen)) {
        return false;
    }

    // Scan LOGS/ once: next name is (highest NNNN.BIN index) + 1.
    // Gaps from PC deletes are never reused (0000,0001,0003 → 0004).
    DIR dir{};
    FRESULT fr = f_opendir(&dir, kLogDir);
    if (fr == FR_NO_PATH) {
        // Directory not created yet — first file is 0000.
        const int printed =
            std::snprintf(path, path_len, "LOGS/0000.BIN");
        return (printed > 0) &&
               (static_cast<uint32_t>(printed) < path_len);
    }
    if (fr != FR_OK) {
        LOG("ERROR: SD f_opendir failed: %d", static_cast<int>(fr));
        return false;
    }

    int32_t highest = -1;
    for (;;) {
        FILINFO info{};
        fr = f_readdir(&dir, &info);

        // Failed to read directory or end of directory.
        if ((fr != FR_OK) || (info.fname[0] == '\0')) {
            break;
        }

        // Skip directories.
        if ((info.fattrib & AM_DIR) != 0U) {
            continue;
        }

        // Expect 8.3 name like "0003.BIN" (FF_USE_LFN == 0).
        uint32_t index = 0U;
        char ext[4] = {};
        if (std::sscanf(info.fname, "%04u.%3s", &index, ext) != 2) {
            continue;
        }
        if ((ext[0] != 'B' && ext[0] != 'b') ||
            (ext[1] != 'I' && ext[1] != 'i') ||
            (ext[2] != 'N' && ext[2] != 'n')) {
            continue;
        }
        if (index > highest) {
            highest = index;
        }
    }
    (void)f_closedir(&dir);

    if (fr != FR_OK) {
        LOG("ERROR: SD f_readdir failed: %d", static_cast<int>(fr));
        return false;
    }

    const uint32_t next = static_cast<uint32_t>(highest + 1);
    if (next >= kMaxLogFileIndex) {
        LOG("ERROR: SD LOGS/ is full (next would be %u)",
            static_cast<unsigned>(next));
        return false;
    }

    // Set the output argument path to the next log file path.
    const int printed = std::snprintf(
        path, path_len, "LOGS/%04u.BIN", static_cast<unsigned>(next));
    return (printed > 0) &&
           (static_cast<uint32_t>(printed) < path_len);
}

extern "C" DSTATUS disk_status(BYTE pdrv) {
    if ((pdrv != 0U) || (g_fatfs_sd_card == nullptr)) {
        return STA_NOINIT;
    }
    return g_fatfs_initialized ? 0U : STA_NOINIT;
}

extern "C" DSTATUS disk_initialize(BYTE pdrv) {
    if ((pdrv != 0U) || (g_fatfs_sd_card == nullptr)) {
        return STA_NOINIT;
    }
    g_fatfs_initialized = g_fatfs_sd_card->Init();
    return g_fatfs_initialized ? 0U : STA_NOINIT;
}

extern "C" DRESULT disk_read(BYTE pdrv,
                             BYTE *buff,
                             LBA_t sector,
                             UINT count) {
    if ((pdrv != 0U) || (g_fatfs_sd_card == nullptr) ||
        !g_fatfs_initialized) {
        return RES_ERROR;
    }

    if ((buff == nullptr) || (count == 0U)) {
        return RES_PARERR;
    }

    for (UINT i = 0U; i < count; ++i) {
        if (!g_fatfs_sd_card->ReadBlock(
                sector + i, buff + (i * SdCard::kBlockSizeBytes))) {
            return RES_ERROR;
        }
    }

    return RES_OK;
}

extern "C" DRESULT disk_write(BYTE pdrv,
                              const BYTE *buff,
                              LBA_t sector,
                              UINT count) {
    if ((pdrv != 0U) || (g_fatfs_sd_card == nullptr) ||
        !g_fatfs_initialized) {
        return RES_ERROR;
    }

    if ((buff == nullptr) || (count == 0U)) {
        return RES_PARERR;
    }

    for (UINT i = 0U; i < count; ++i) {
        if (!g_fatfs_sd_card->WriteBlock(
                sector + i, buff + (i * SdCard::kBlockSizeBytes))) {
            return RES_ERROR;
        }
    }

    return RES_OK;
}

extern "C" DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    if ((pdrv != 0U) || (g_fatfs_sd_card == nullptr) ||
        !g_fatfs_initialized) {
        return RES_NOTRDY;
    }

    switch (cmd) {
    case CTRL_SYNC:
        // Writes wait for programming busy before returning.
        return RES_OK;
    case GET_SECTOR_SIZE:
        if (buff == nullptr) {
            return RES_PARERR;
        }
        *static_cast<WORD *>(buff) =
            static_cast<WORD>(SdCard::kBlockSizeBytes);
        return RES_OK;
    default:
        (void)buff;
        return RES_PARERR;
    }
}
