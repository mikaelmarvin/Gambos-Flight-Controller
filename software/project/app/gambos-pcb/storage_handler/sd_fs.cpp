/**
 * @file sd_fs.cpp
 * @brief FatFs on SdCard — stub until export path exists.
 */

#include "sd_fs.hpp"

SdCardFileSystem::SdCardFileSystem(SdCard &sd) : _sd(sd) {}

bool SdCardFileSystem::Initialize(void) {
    (void)_sd;
    // Mount / register happens when post-flight export is wired.
    return true;
}
