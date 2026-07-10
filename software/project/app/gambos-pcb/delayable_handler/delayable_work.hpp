#ifndef DELAYABLE_WORK_HPP
#define DELAYABLE_WORK_HPP

#include "FreeRTOS.h"
#include "queue.h"

#include "log.hpp"
#include <array>
#include <functional>
#include <stdint.h>

inline constexpr uint8_t kMaxDelayableWorkItems = 10;
inline constexpr uint8_t kNotRegistered = UINT8_MAX;

class DelayableWork {
  public:
    DelayableWork() = default;
    ~DelayableWork() = default;

    bool Initialize(std::function<void(void)> function);
    bool Schedule(uint32_t execute_after_ms);
    bool ScheduleOnce(uint32_t execute_after_ms);
    bool Cancel(void);
    bool Remove(void);

    static void Start(void);

  private:
    static void TaskFunction(void *pvParameters);
    static bool RemoveRegisteredItemAt(uint8_t index);
    static bool Enqueue(DelayableWork *work_item);

    std::function<void(void)> _function;
    TickType_t _execute_at_tick = 0;
    bool _is_pending = false;
    bool _pending_remove = false;
    bool _remove_after_run = false;
    uint8_t _registry_index = kNotRegistered;

    inline static bool _task_started = false;
    inline static std::array<DelayableWork *, kMaxDelayableWorkItems>
        _registered_items{};
    inline static uint8_t _registered_count = 0;
    inline static StaticQueue_t _pending_work_queue_state;
    inline static QueueHandle_t _pending_work_queue_handle;
    inline static uint8_t
        _pending_work_queue_storage[kMaxDelayableWorkItems *
                                    sizeof(void *)];
};

#endif /* DELAYABLE_WORK_HPP */
