#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdint>

inline constexpr uint8_t kSettingsVersion = 1U;

struct Settings {
    uint8_t version{kSettingsVersion};
};

#endif /* SETTINGS_HPP */