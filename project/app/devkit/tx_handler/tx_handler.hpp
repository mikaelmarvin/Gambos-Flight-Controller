#ifndef TX_HANDLER_HPP
#define TX_HANDLER_HPP

#include "nrf24l01p.hpp"

class TxHandler {
  public:
    TxHandler() = default;
    ~TxHandler() = default;

    bool Initialize(void);
    void Start(void);

  private:
    static void TaskFunction(void *pvParameters);

    Nrf24l01p nrf24l01p_;
};

#endif /* TX_HANDLER_HPP */
