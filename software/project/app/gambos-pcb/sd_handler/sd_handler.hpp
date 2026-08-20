#ifndef SD_HANDLER_HPP
#define SD_HANDLER_HPP

#include "ff.h"
#include "sd_card.hpp"

class SdHandler {
  public:
    explicit SdHandler(SdCard &sd);

    bool Initialize(void);
    bool IsMounted(void) const;

  private:
    bool WriteSmokeTestFile(void);

    SdCard &_sd;
    FATFS _fs{};
    bool _mounted{false};
};

#endif /* SD_HANDLER_HPP */
