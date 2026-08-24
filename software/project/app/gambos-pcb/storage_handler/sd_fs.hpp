#ifndef SD_FS_HPP
#define SD_FS_HPP

#include "sd_card.hpp"

// FatFs volume on the SPI SD card. Stub until post-flight export.
class SdCardFileSystem {
  public:
    explicit SdCardFileSystem(SdCard &sd);

    bool Initialize(void);

  private:
    SdCard &_sd;
};

#endif // SD_FS_HPP
