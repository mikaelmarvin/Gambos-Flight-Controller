#ifndef TX_HANDLER_HPP
#define TX_HANDLER_HPP

class TxHandler {
  public:
    TxHandler() = default;
    ~TxHandler() = default;

    bool Initialize(void);
    void Start(void);

  private:
    static void TaskFunction(void *pvParameters);
};

#endif /* TX_HANDLER_HPP */
