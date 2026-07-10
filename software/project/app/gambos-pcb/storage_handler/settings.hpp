#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdint>

inline constexpr uint8_t kSettingsVersion = 1U;

struct Settings {
    uint8_t version{kSettingsVersion};
    uint32_t test_counter{0U};
    uint32_t last_write_tick_ms{0U};
};

#endif /* SETTINGS_HPP */