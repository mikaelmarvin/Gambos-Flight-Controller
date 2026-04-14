#ifndef BUTTON_HANDLER_HPP
#define BUTTON_HANDLER_HPP

#include "FreeRTOS.h"
#include "semphr.h"

class ButtonHandler {
  public:
    ButtonHandler() = default;
    ~ButtonHandler() = default;

    bool Initialize(void);
    void Start(void);

    static void CallbackFromISR(void);

  private:
    static void TaskFunction(void *pvParameters);

    static StaticSemaphore_t button_semaphore_buffer;
    static SemaphoreHandle_t button_semaphore;
};

#endif /* BUTTON_HANDLER_HPP */
