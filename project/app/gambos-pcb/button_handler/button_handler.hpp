#ifndef BUTTON_HANDLER_HPP
#define BUTTON_HANDLER_HPP

#include "delayable_handler/delayable_work.hpp"

#include "FreeRTOS.h"

class ButtonHandler {
  public:
    ButtonHandler() = default;
    ~ButtonHandler() = default;

    bool Initialize(void);
    void Start(void);

    static void CallbackFromISR(void);

    inline static ButtonHandler *_instance{nullptr};

  private:
    static void TaskFunction(void *pvParameters);

    DelayableWork _delayed_press_work = {};
    TaskHandle_t _task_handle = nullptr;
};

#endif /* BUTTON_HANDLER_HPP */
