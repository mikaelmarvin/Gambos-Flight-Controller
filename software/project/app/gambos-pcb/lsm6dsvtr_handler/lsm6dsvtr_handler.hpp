#ifndef LSM6DSVTR_HANDLER_HPP
#define LSM6DSVTR_HANDLER_HPP

#include "lsm6dsvtr.hpp"

/**
 * One task per LSM6DSVTR chip: single I2C burst, accel published every cycle,
 * gyro published on a slower schedule (decimation).
 */
class Lsm6dsvtrHandler {
  public:
    explicit Lsm6dsvtrHandler(Lsm6dsvtr &device);

    bool Initialize(void);
    void Start(void);

  private:
    static void TaskFunction(void *pvParameters);

    bool ReadAndPublish(void);

    Lsm6dsvtr &_device;
    uint32_t _loop_count{0U};
};

#endif /* LSM6DSVTR_HANDLER_HPP */
