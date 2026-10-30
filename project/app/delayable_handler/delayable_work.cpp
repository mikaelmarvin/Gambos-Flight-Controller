/**
 * @file delayable_work.cpp
 * @brief Delayable work scheduler (FreeRTOS task + pointer queue).
 *
 * Each DelayableWork instance must outlive any scheduled execution
 * (static storage, pool, or long-lived member — not a stack local).
 */

#include "delayable_work.hpp"
#include "log.hpp"
#include "task.h"

namespace {

constexpr uint32_t kDelayableWorkTaskStackSize = 512U;
constexpr uint32_t kDelayableWorkTaskPriority =
    (tskIDLE_PRIORITY + 1U);

} // namespace

bool DelayableWork::Initialize(std::function<void(void)> function) {
    _function = std::move(function);
    _is_pending = false;
    _execute_at_tick = 0;
    _pending_remove = false;
    _remove_after_run = false;
    _registry_index = kNotRegistered;
    return true;
}

bool DelayableWork::Schedule(uint32_t execute_after_ms) {
    _execute_at_tick =
        xTaskGetTickCount() + pdMS_TO_TICKS(execute_after_ms);
    _is_pending = true;
    return Enqueue(this);
}

bool DelayableWork::ScheduleOnce(uint32_t execute_after_ms) {
    _remove_after_run = true;
    if (!Schedule(execute_after_ms)) {
        _remove_after_run = false;
        return false;
    }

    return true;
}

bool DelayableWork::Cancel(void) {
    _is_pending = false;
    _remove_after_run = false;
    return true;
}

bool DelayableWork::Remove(void) {
    _is_pending = false;
    _pending_remove = true;

    if (!Enqueue(this)) {
        _pending_remove = false;
        return false;
    }

    return true;
}

void DelayableWork::Start(void) {
    if (_pending_work_queue_handle == nullptr) {
        _pending_work_queue_handle =
            xQueueCreateStatic(kMaxDelayableWorkItems,
                               sizeof(void *),
                               _pending_work_queue_storage,
                               &_pending_work_queue_state);
    }

    if (_task_started) {
        return;
    }

    _registered_items.fill(nullptr);
    _registered_count = 0;

    configASSERT(xTaskCreate(&DelayableWork::TaskFunction,
                             "delayable_work",
                             kDelayableWorkTaskStackSize,
                             nullptr,
                             kDelayableWorkTaskPriority,
                             nullptr) == pdPASS);
    _task_started = true;
}

void DelayableWork::TaskFunction(void *pvParameters) {
    (void)pvParameters;

    while (true) {
        TickType_t current_time = xTaskGetTickCount();

        // Step 1: Find the next work item to execute.
        // The next work item to execute is the one that has the
        // earliest execution time. If none is found, the task waits
        // until a work item is enqueued.
        DelayableWork *next_work_item = nullptr;
        TickType_t next_wakeup_delay = portMAX_DELAY;
        for (uint8_t i = 0; i < _registered_count; i++) {
            next_work_item = _registered_items[i];
            if (next_work_item == nullptr ||
                !next_work_item->_is_pending) {
                continue;
            }

            // If the work item is ready to execute, we set the next
            // wakeup delay to 0 and break out of the loop. This might
            // happen if the User scheduled a work item with a delay
            // of 0 milliseconds.
            if (next_work_item->_execute_at_tick <= current_time) {
                next_wakeup_delay = 0;
                break;
            }

            // Here is where we find the lowest remaining time of all
            // registered work items. Once found, the xQueueReceive()
            // call will wait for the remaining time, that is, if no
            // additional work item is enqueued before the remaining
            // time expires. Also, the work item the task is
            // waiting for is tracked. This is done to avoid having to
            // search for the work item again in the execution step.
            const TickType_t remaining =
                next_work_item->_execute_at_tick - current_time;
            if (remaining < next_wakeup_delay) {
                next_wakeup_delay = remaining;
            }
        }

        // Step 2: Wait for a work item to be enqueued.
        DelayableWork *incoming_item = nullptr;
        if (xQueueReceive(_pending_work_queue_handle,
                          &incoming_item,
                          next_wakeup_delay) == pdPASS) {
            if (incoming_item == nullptr) {
                LOG("Error: Something went wrong while receiving a "
                    "work item from the queue.");
                continue;
            }

            // If the work item is marked for removal, we remove it
            // from the registered items list.
            if (incoming_item->_pending_remove) {
                incoming_item->_pending_remove = false;

                // If the work item is registered, we remove it from
                // the registered items list. Safety.
                if (incoming_item->_registry_index <
                        _registered_count &&
                    _registered_items[incoming_item
                                          ->_registry_index] ==
                        incoming_item) {
                    RemoveRegisteredItemAt(
                        incoming_item->_registry_index);
                }
                continue;
            }

            // The work item is not registered, so we add it to the
            // registered items list.
            if (incoming_item->_registry_index == kNotRegistered) {
                if (_registered_count >= kMaxDelayableWorkItems) {
                    LOG("ERROR: Max delayable work items reached, "
                        "dropping work item.");
                    continue;
                }

                // The _registered_count follows the index of the next
                // available slot in the registered items list.
                _registered_items[_registered_count] = incoming_item;
                incoming_item->_registry_index = _registered_count;
                _registered_count++;
            }
        }

        // No other work was enqueued and the task is done waiting for
        // the work item to execute. Execute the tracked work item.
        else {
            if (next_work_item->_function) {
                next_work_item->_function();
            }

            // The work item is no longer pending.
            next_work_item->_is_pending = false;

            // If the work item is marked for removal after execution,
            // we remove it from the registered items list.
            if (next_work_item->_remove_after_run) {
                next_work_item->_remove_after_run = false;
                RemoveRegisteredItemAt(
                    next_work_item->_registry_index);
            }
        }
    }
}

bool DelayableWork::RemoveRegisteredItemAt(uint8_t index) {
    if (index >= _registered_count) {
        return false;
    }

    // Get pointers to work item being removed and the last work
    // item in the list. The goal is to replace the work item
    // being removed with the last work item in the list. This is
    // done to avoid having to shift all the work items in the
    // list to maintain a dense array without shifting work items.
    DelayableWork *const removed = _registered_items[index];
    const uint8_t last_index =
        static_cast<uint8_t>(_registered_count - 1);
    DelayableWork *const moved = _registered_items[last_index];

    // Update the moved (last) work item's registry index to the
    // index of the work item being removed.
    _registered_items[index] = moved;
    if (moved != nullptr) {
        moved->_registry_index = index;
    }

    _registered_items[last_index] = nullptr;
    _registered_count--;

    // Update the removed work item's registry index to not
    // registered.
    if (removed != nullptr) {
        removed->_registry_index = kNotRegistered;
    }

    return true;
}

bool DelayableWork::Enqueue(DelayableWork *work_item) {
    if (_pending_work_queue_handle == nullptr) {
        LOG("ERROR: DelayableWork not started");
        return false;
    }

    if (xQueueSend(_pending_work_queue_handle, &work_item, 0) !=
        pdPASS) {
        LOG("ERROR: Failed to enqueue delayable work");
        return false;
    }

    return true;
}
