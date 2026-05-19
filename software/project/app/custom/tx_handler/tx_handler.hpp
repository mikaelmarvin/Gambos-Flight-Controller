#ifndef TX_HANDLER_HPP
#define TX_HANDLER_HPP

#include "nrf24l01p.hpp"
#include "spi.h"

class TxHandler {
  public:
    TxHandler() = default;
    ~TxHandler() = default;

    bool Initialize(void);
    void Start(void);

  private:
    static void TaskFunction(void *pvParameters);

    Nrf24l01p _nrf24l01p{&hspi2};
};

#endif /* TX_HANDLER_HPP */
