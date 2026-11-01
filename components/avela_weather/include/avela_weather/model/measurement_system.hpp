// The units the weather is shown in (skin API): metric = degrees Celsius and km/h, imperial = degrees Fahrenheit and
// mph. Chosen on the setup page; the values in a WeatherReport are already in these units. Platform-free.
#pragma once

#include <cstdint>

namespace avela {

enum class MeasurementSystem : uint8_t { metric, imperial };

}  // namespace avela
