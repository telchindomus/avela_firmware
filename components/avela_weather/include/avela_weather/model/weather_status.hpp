// Where the weather service is, as the weather screen shows it (skin API). Platform-free.
#pragma once

#include <cstdint>

namespace avela {

enum class WeatherStatus : uint8_t {
    not_configured,       // no place set on the setup page
    waiting_for_network,  // a place is set, the radio isn't on the home network
    loading,              // asking Open-Meteo
    ready,                // the values are fresh
    place_not_found,      // Open-Meteo knows no place of that name
    failed,               // the last update failed (the values, if any, are older)
};

}  // namespace avela
