#ifndef STORAGE_HANDLER_HPP
#define STORAGE_HANDLER_HPP

#include "at25sf128a.hpp"

class StorageHandler {
  public:
    StorageHandler() = default;

    bool Initialize(void);
    void Start(void);

  private:
    static void TaskFunction(void *pvParameters);

    At25sf128a _device{};
};

#endif /* STORAGE_HANDLER_HPP */
