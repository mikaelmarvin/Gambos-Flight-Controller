#ifndef IIS2MDCTR_HANDLER_HPP
#define IIS2MDCTR_HANDLER_HPP

#include "iis2mdctr.hpp"

class Iis2mdctrHandler {
  public:
    Iis2mdctrHandler() = default;

    bool Initialize(void);
    void Start(void);

  private:
    static void TaskFunction(void *pvParameters);

    bool ReadAndPublish(void);

    Iis2mdctr _device{};
};

#endif /* IIS2MDCTR_HANDLER_HPP */
