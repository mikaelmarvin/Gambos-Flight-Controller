/**
 * @file flash_fs.cpp
 * @brief LittleFS on AT25SF128A — mount + settings/log file helpers.
 */

#include "flash_fs.hpp"
#include "log.hpp"

#include <cstring>

namespace {

constexpr lfs_size_t kLfsBlockCount = static_cast<lfs_size_t>(
    At25sf128a::kCapacityBytes / At25sf128a::kSectorSizeBytes);

constexpr char kSettingsFilePath[] = "/settings.bin";
constexpr char kLogFilePath[] = "/logs/0000.bin";

int LfsBdRead(const lfs_config *c,
              lfs_block_t block,
              lfs_off_t off,
              void *buffer,
              lfs_size_t size) {
    At25sf128a *flash = static_cast<At25sf128a *>(c->context);
    const uint32_t addr = block * c->block_size + off;
    return flash->Read(addr, static_cast<uint8_t *>(buffer), size)
               ? LFS_ERR_OK
               : LFS_ERR_IO;
}

int LfsBdProg(const lfs_config *c,
              lfs_block_t block,
              lfs_off_t off,
              const void *buffer,
              lfs_size_t size) {
    At25sf128a *flash = static_cast<At25sf128a *>(c->context);
    const uint32_t addr = block * c->block_size + off;
    return flash->Write(
               addr, static_cast<const uint8_t *>(buffer), size)
               ? LFS_ERR_OK
               : LFS_ERR_IO;
}

int LfsBdErase(const lfs_config *c, lfs_block_t block) {
    At25sf128a *flash = static_cast<At25sf128a *>(c->context);
    const uint32_t addr = block * c->block_size;
    return flash->Erase(addr) ? LFS_ERR_OK : LFS_ERR_IO;
}

int LfsBdSync(const lfs_config *c) {
    At25sf128a *flash = static_cast<At25sf128a *>(c->context);
    return flash->EnsureIdle() ? LFS_ERR_OK : LFS_ERR_IO;
}

} // namespace

FlashFileSystem::FlashFileSystem(At25sf128a &flash) : _flash(flash) {}

bool FlashFileSystem::Initialize(void) {
    std::memset(_file_buffer, 0, sizeof(_file_buffer));
    std::memset(&_file_config, 0, sizeof(_file_config));
    _file_config.buffer = _file_buffer;

    std::memset(&_lfs_cfg, 0, sizeof(_lfs_cfg));
    _lfs_cfg.context = &_flash;
    _lfs_cfg.read = LfsBdRead;
    _lfs_cfg.prog = LfsBdProg;
    _lfs_cfg.erase = LfsBdErase;
    _lfs_cfg.sync = LfsBdSync;
    _lfs_cfg.read_size = At25sf128a::kPageProgramBytes;
    _lfs_cfg.prog_size = At25sf128a::kPageProgramBytes;
    _lfs_cfg.block_size = At25sf128a::kSectorSizeBytes;
    _lfs_cfg.block_count = kLfsBlockCount;
    _lfs_cfg.cache_size = kLfsCacheSize;
    _lfs_cfg.lookahead_size = kLfsLookaheadSize;
    _lfs_cfg.block_cycles = 500;
    _lfs_cfg.read_buffer = _lfs_read_buffer;
    _lfs_cfg.prog_buffer = _lfs_prog_buffer;
    _lfs_cfg.lookahead_buffer = _lfs_lookahead_buffer;

    int8_t err = lfs_mount(&_lfs, &_lfs_cfg);
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

bool FlashFileSystem::OpenSettingsForWrite(void) {
    if (_settings_file_is_open) {
        return true;
    }

    const int8_t err =
        lfs_file_opencfg(&_lfs,
                         &_settings_file,
                         kSettingsFilePath,
                         LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC,
                         &_file_config);
    if (err == LFS_ERR_OK) {
        _settings_file_is_open = true;
        return true;
    }

    LOG("ERROR: Failed to open settings file for write: %d", err);
    return false;
}

bool FlashFileSystem::OpenSettingsForRead(void) {
    if (_settings_file_is_open) {
        return true;
    }

    const int8_t err = lfs_file_opencfg(&_lfs,
                                        &_settings_file,
                                        kSettingsFilePath,
                                        LFS_O_RDONLY,
                                        &_file_config);
    if (err == LFS_ERR_OK) {
        _settings_file_is_open = true;
        return true;
    }

    LOG("ERROR: Failed to open settings file for read: %d", err);
    return false;
}

bool FlashFileSystem::CloseSettings(void) {
    if (!_settings_file_is_open) {
        return true;
    }

    const int8_t err = lfs_file_close(&_lfs, &_settings_file);
    if (err == LFS_ERR_OK) {
        _settings_file_is_open = false;
        return true;
    }

    LOG("ERROR: Failed to close settings file: %d", err);
    return false;
}

bool FlashFileSystem::OpenLogsForWrite(void) {
    if (_logs_file_is_open) {
        return true;
    }

    const int8_t err =
        lfs_file_opencfg(&_lfs,
                         &_log_file,
                         kLogFilePath,
                         LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND,
                         &_file_config);
    if (err == LFS_ERR_OK) {
        _logs_file_is_open = true;
        return true;
    }

    LOG("ERROR: Failed to open logs file for write: %d", err);
    return false;
}

bool FlashFileSystem::OpenLogsForRead(void) {
    if (_logs_file_is_open) {
        return true;
    }

    const int8_t err = lfs_file_opencfg(
        &_lfs, &_log_file, kLogFilePath, LFS_O_RDONLY, &_file_config);
    if (err == LFS_ERR_OK) {
        _logs_file_is_open = true;
        return true;
    }

    LOG("ERROR: Failed to open logs file for read: %d", err);
    return false;
}

bool FlashFileSystem::CloseLogs(void) {
    if (!_logs_file_is_open) {
        return true;
    }

    const int8_t err = lfs_file_close(&_lfs, &_log_file);
    if (err == LFS_ERR_OK) {
        _logs_file_is_open = false;
        return true;
    }

    LOG("ERROR: Failed to close logs file: %d", err);
    return false;
}

bool FlashFileSystem::SyncLogs(void) {
    const int8_t err = lfs_file_sync(&_lfs, &_log_file);
    if (err == LFS_ERR_OK) {
        return true;
    }

    LOG("ERROR: Failed to sync logs file: %d", err);
    return false;
}

bool FlashFileSystem::WriteSettings(const Settings &settings) {
    const lfs_ssize_t written = lfs_file_write(
        &_lfs, &_settings_file, &settings, sizeof(settings));
    return written == static_cast<lfs_ssize_t>(sizeof(settings));
}

bool FlashFileSystem::ReadSettings(Settings &settings) {
    const lfs_ssize_t bytes_read = lfs_file_read(
        &_lfs, &_settings_file, &settings, sizeof(settings));
    return bytes_read == static_cast<lfs_ssize_t>(sizeof(settings));
}

bool FlashFileSystem::WriteLogs(const uint8_t *data, uint32_t size) {
    if ((data == nullptr) || (size == 0U)) {
        return false;
    }

    const lfs_ssize_t written =
        lfs_file_write(&_lfs, &_log_file, data, size);
    return written == static_cast<lfs_ssize_t>(size);
}

int32_t FlashFileSystem::ReadLogs(uint8_t *data, uint32_t size) {
    if ((data == nullptr) || (size == 0U) || !_logs_file_is_open) {
        return -1;
    }

    const lfs_ssize_t bytes_read =
        lfs_file_read(&_lfs, &_log_file, data, size);
    if (bytes_read < 0) {
        LOG("ERROR: Failed to read logs file: %d",
            static_cast<int>(bytes_read));
        return -1;
    }

    return static_cast<int32_t>(bytes_read);
}

bool FlashFileSystem::ResetLogs(void) {
    if (!CloseLogs()) {
        return false;
    }

    const int8_t err = lfs_remove(&_lfs, kLogFilePath);
    if ((err != LFS_ERR_OK) && (err != LFS_ERR_NOENT)) {
        LOG("ERROR: Failed to remove logs file: %d", err);
        return false;
    }

    // Recreate empty /logs/0000.bin for the next logging session.
    if (!OpenLogsForWrite()) {
        return false;
    }
    return CloseLogs();
}
