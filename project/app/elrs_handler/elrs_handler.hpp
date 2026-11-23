#ifndef ELRS_HANDLER_HPP
#define ELRS_HANDLER_HPP

#include "FreeRTOS.h"
#include "task.h"

#include "elrs_receiver.hpp"

class ElrsHandler {
  public:
    explicit ElrsHandler(ElrsReceiver &receiver) : _elrs_receiver(receiver) {}
    ~ElrsHandler() = default;

    bool Initialize(void);
    void Start(void);

  private:
    static void TaskFunction(void *pvParameters);

    TaskHandle_t _task_handle = nullptr;
    ElrsReceiver &_elrs_receiver;
};

#endif /* ELRS_HANDLER_HPP */
