#ifndef SENSING_HANDLER_HPP
#define SENSING_HANDLER_HPP

#include "bmp384.hpp"
#include "iis2mdctr.hpp"
#include "lsm6dsvtr.hpp"

/**
 * One FreeRTOS task for all I2C flight sensors (IMU, mag, baro).
 * Devices stay separate; scheduling is shared so bus access is
 * ordered without relying on three contending tasks.
 */
class SensingHandler {
  public:
    SensingHandler(Lsm6dsvtr &imu, Iis2mdctr &mag, Bmp384 &baro);

    bool Initialize(void);
    void Start(void);

  private:
    static void TaskFunction(void *pvParameters);

    bool ReadAndPublishImu(void);
    bool ReadAndPublishMag(void);
    bool ReadAndPublishBaro(void);

    Lsm6dsvtr &_imu;
    Iis2mdctr &_mag;
    Bmp384 &_baro;
    uint32_t _loop_count{0U};
};

#endif /* SENSING_HANDLER_HPP */
