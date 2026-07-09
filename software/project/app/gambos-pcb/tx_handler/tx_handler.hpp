#ifndef TX_HANDLER_HPP
#define TX_HANDLER_HPP

#include "nrf24l01p.hpp"

class TxHandler {
  public:
    explicit TxHandler(Nrf24l01p &radio);

    bool Initialize(void);
    void Start(void);

  private:
    static void TaskFunction(void *pvParameters);

    Nrf24l01p &_radio;
};

#endif /* TX_HANDLER_HPP */
