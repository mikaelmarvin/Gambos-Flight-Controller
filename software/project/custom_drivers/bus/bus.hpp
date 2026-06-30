#ifndef BUS_HPP
#define BUS_HPP

#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include <cstdint>

struct CsPin {
    GPIO_TypeDef *port{nullptr};
    uint16_t pin{0U};
};

/** RAII chip-select assert (active low). */
class CsAssert {
  public:
    explicit CsAssert(CsPin cs);
    ~CsAssert();

    CsAssert(const CsAssert &) = delete;
    CsAssert &operator=(const CsAssert &) = delete;

  private:
    CsPin _cs{};
};

/** One instance per physical SPI peripheral (hspi1, hspi2). */
class SpiBus {
  public:
    bool Init(SPI_HandleTypeDef *hspi);
    bool IsInitialized(void) const;

    bool TransmitDma(const uint8_t *tx,
                     uint16_t len,
                     TickType_t timeout);
    bool ReceiveDma(uint8_t *rx, uint16_t len, TickType_t timeout);
    bool TransmitReceiveDma(const uint8_t *tx,
                            uint8_t *rx,
                            uint16_t len,
                            TickType_t timeout);

    void SignalIfHandle(SPI_HandleTypeDef *hspi);

  private:
    bool EnsureDmaSemaphore(void);
    bool EnsureBusMutex(void);
    bool BeginDma(void);
    bool WaitDma(TickType_t timeout);
    void SignalFromIsr(void);

    SPI_HandleTypeDef *_hspi{nullptr};

    // Ensures blocking a task until the DMA is complete.
    StaticSemaphore_t _dma_sem_buffer{};
    SemaphoreHandle_t _dma_sem{nullptr};

    // Ensures protecting the bus from access by multiple tasks.
    StaticSemaphore_t _bus_mutex_buffer{};
    SemaphoreHandle_t _bus_mutex{nullptr};
};

/** One instance per physical I2C peripheral (hi2c1). */
class I2cBus {
  public:
    bool Init(I2C_HandleTypeDef *hi2c);
    bool IsInitialized(void) const;

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

    void SignalIfHandle(I2C_HandleTypeDef *hi2c);

  private:
    bool EnsureDmaSemaphore(void);
    bool EnsureBusMutex(void);
    bool BeginDma(void);
    bool WaitDma(TickType_t timeout);
    void SignalFromIsr(void);

    I2C_HandleTypeDef *_hi2c{nullptr};

    // Ensures blocking a task until the DMA is complete.
    StaticSemaphore_t _dma_sem_buffer{};
    SemaphoreHandle_t _dma_sem{nullptr};

    // Ensures protecting the bus from access by multiple tasks.
    StaticSemaphore_t _bus_mutex_buffer{};
    SemaphoreHandle_t _bus_mutex{nullptr};
};

// Singleton instances for each physical bus.
SpiBus &Spi1(void);
SpiBus &Spi2(void);
I2cBus &I2c1(void);

#endif /* BUS_HPP */
