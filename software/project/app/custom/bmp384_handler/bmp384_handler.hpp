#ifndef BMP384_HANDLER_HPP
#define BMP384_HANDLER_HPP

#include "bmp384.hpp"

/** One task for BMP384 — pressure and temperature from one read. */
class Bmp384Handler {
  public:
    Bmp384Handler() = default;

    bool Initialize(void);
    void Start(void);

  private:
    static void TaskFunction(void *pvParameters);

    bool ReadAndPublish(void);

    Bmp384 _device{};
};

#endif /* BMP384_HANDLER_HPP */
