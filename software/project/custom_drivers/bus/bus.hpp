#ifndef BUS_HPP
#define BUS_HPP

#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include <cstdint>

#ifndef HAL_I2C_MODULE_ENABLED
struct I2C_HandleTypeDef;
#endif

// One instance per physical SPI or I2C peripheral (hspi1, hspi2, hi2c1).
class Bus {
  public:
    enum class Kind : uint8_t {
        Uninitialized = 0U,
        Spi = 1U,
        I2c = 2U,
    };

    bool Init(SPI_HandleTypeDef *spi);
    bool Init(I2C_HandleTypeDef *hi2c);

    bool IsInitialized(void) const;
    Kind GetKind(void) const;

    SPI_HandleTypeDef *Spi(void) const;
    I2C_HandleTypeDef *I2c(void) const;

    bool BeginDma(void);
    bool WaitDma(TickType_t timeout);
    void SignalFromIsr(void);

    bool MemRead(uint16_t dev_addr_7bit,
                 uint16_t reg,
                 uint8_t *data,
                 uint16_t len,
                 TickType_t timeout);
    bool MemWrite(uint16_t dev_addr_7bit,
                  uint16_t reg,
                  const uint8_t *data,
                  uint16_t len,
                  TickType_t timeout);

  private:
    bool EnsureDmaSemaphore(void);

    Kind _kind{Kind::Uninitialized};
    SPI_HandleTypeDef *_spi{nullptr};
    I2C_HandleTypeDef *_hi2c{nullptr};

    StaticSemaphore_t _dma_sem_buffer{};
    SemaphoreHandle_t _dma_sem{nullptr};

    StaticSemaphore_t _bus_mutex_buffer{};
    SemaphoreHandle_t _bus_mutex{nullptr};
};

#endif /* BUS_HPP */
