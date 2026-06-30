#ifndef IIS2MDCTR_HANDLER_HPP
#define IIS2MDCTR_HANDLER_HPP

#include "iis2mdctr.hpp"

class Iis2mdctrHandler {
  public:
    explicit Iis2mdctrHandler(Iis2mdctr &device);

    bool Initialize(void);
    void Start(void);

  private:
    static void TaskFunction(void *pvParameters);

    bool ReadAndPublish(void);

    Iis2mdctr &_device;
};

#endif /* IIS2MDCTR_HANDLER_HPP */
