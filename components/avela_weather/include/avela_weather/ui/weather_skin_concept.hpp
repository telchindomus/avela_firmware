// Skin authors: components/avela_weather/docs/developer/SKIN.md describes every setter (values, ranges, controls).
// What the weather screen shows, as its skin must take it: the whole report, one value that changes
// every few minutes at most (WeatherReport: the service's state, the place, the units, the current weather, today's
// high and low, the next days), and the radio's host name (where the place is set: http://<host name>.local). Once
// shown (draw()), the setters redraw at once if the report's revision or the name differs from what is shown; the
// screen calls them each frame.
#pragma once

#include <concepts>
#include <string_view>

#include "avela_weather/model/weather_report.hpp"

namespace avela {

template <typename T>
concept WeatherSkinConcept = requires(T& skin, const WeatherReport& report, std::string_view host_name) {
    { skin.set_weather(report) } -> std::same_as<void>;
    { skin.set_host_name(host_name) } -> std::same_as<void>;
    { skin.draw() } -> std::same_as<void>;
};

}  // namespace avela
