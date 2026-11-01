// What the sky does, as the skins show it (skin API): Open-Meteo's WMO weather codes in a few groups a small icon can
// tell apart. Platform-free.
#pragma once

#include <cstdint>

namespace avela {

enum class WeatherCondition : uint8_t {
    clear,
    partly_cloudy,
    cloudy,
    fog,
    drizzle,
    rain,
    snow,
    thunderstorm,
};

}  // namespace avela
