/**
 * @file sd_handler.cpp
 * @brief Wires the SPI SD card into FatFs (register + mount).
 */

#include "sd_handler.hpp"

#include "log.hpp"
#include "main.h"

#include "stm32f4xx_hal_gpio.h"

#include <cstring>

namespace {

constexpr char kSdVolumePath[] = "0:";
// FF_USE_LFN is 0 → 8.3 names only (max 8 + '.' + 3).
constexpr char kSmokeTestPath[] = "SMOKE.TXT";
constexpr char kSmokeTestPayload[] =
    "gambos SD smoke test\r\n"
    "If you can read this on a PC, FatFs write works.\r\n";

// FIL embeds a 512-byte window when FF_FS_TINY==0; keep it off the
// app_startup stack.
FIL g_smoke_file{};

} // namespace

SdHandler::SdHandler(SdCard &sd) : _sd(sd) {}

bool SdHandler::WriteSmokeTestFile(void) {
    UINT bytes_written = 0U;

    FRESULT fr = f_open(&g_smoke_file,
                        kSmokeTestPath,
                        FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        LOG("ERROR: SD f_open failed: %d\r\n", static_cast<int>(fr));
        return false;
    }

    fr = f_write(&g_smoke_file,
                 kSmokeTestPayload,
                 static_cast<UINT>(std::strlen(kSmokeTestPayload)),
                 &bytes_written);
    if (fr != FR_OK) {
        LOG("ERROR: SD f_write failed: %d\r\n", static_cast<int>(fr));
        (void)f_close(&g_smoke_file);
        return false;
    }

    fr = f_close(&g_smoke_file);
    if (fr != FR_OK) {
        LOG("ERROR: SD f_close failed: %d\r\n", static_cast<int>(fr));
        return false;
    }

    LOG("SD: wrote %u bytes to %s\r\n",
        static_cast<unsigned>(bytes_written),
        kSmokeTestPath);
    return true;
}

bool SdHandler::Initialize(void) {
    // Cube leaves SD_CS low; idle high before any SPI traffic.
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);

    RegisterSdCardForFatFs(_sd);

    const FRESULT fr = f_mount(&_fs, kSdVolumePath, 1);
    if (fr != FR_OK) {
        LOG("ERROR: SD f_mount failed: %d\r\n", static_cast<int>(fr));
        _mounted = false;
        return false;
    }

    _mounted = true;
    LOG("SD: FatFs mounted on %s\r\n", kSdVolumePath);

    return WriteSmokeTestFile();
}

bool SdHandler::IsMounted(void) const { return _mounted; }
