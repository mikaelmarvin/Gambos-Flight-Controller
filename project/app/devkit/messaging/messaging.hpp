#ifndef MESSAGING_HPP
#define MESSAGING_HPP

#include <array>
#include <stdint.h>

// Warning: Must keep an eye on how many subscribers are subscribed to
// a specific topic.
inline constexpr uint8_t kMaxSubscribersPerTopic = 4;

template <typename Topic>
using CallbackFunction = void (*)(const Topic &topic);

template <typename Topic> struct SubscriberTable {
    inline static std::array<CallbackFunction<Topic>,
                             kMaxSubscribersPerTopic>
        subscribers{};
};

class Messaging {
  public:
    Messaging() = default;
    ~Messaging() = default;

    template <typename Topic>
    static bool Publish(const Topic &topic) {
        for (uint8_t i = 0; i < kMaxSubscribersPerTopic; i++) {
            if (SubscriberTable<Topic>::subscribers[i] != nullptr) {
                SubscriberTable<Topic>::subscribers[i](topic);
            }
        }

        return true;
    }

    template <typename Topic>
    static bool Subscribe(CallbackFunction<Topic> callback) {
        for (uint8_t i = 0; i < kMaxSubscribersPerTopic; i++) {
            if (SubscriberTable<Topic>::subscribers[i] == nullptr) {
                SubscriberTable<Topic>::subscribers[i] = callback;
                return true;
            }
        }

        return false;
    }
};

namespace topics {

struct ButtonInfo {
    uint8_t button_id;
    uint8_t button_state;
};

} // namespace topics

#endif /* MESSAGING_HPP */
