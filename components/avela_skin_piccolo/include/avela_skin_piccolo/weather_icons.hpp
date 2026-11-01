// The piccolo skin's weather icons, drawn from shapes at any size (the weather screen's big one, the forecast row's and
// the tile's small ones): sun or moon, clouds, fog, drizzle, rain, snow, thunderstorm.
#pragma once

#include <cstdint>

#include "avela_ui/canvas.hpp"
#include "avela_weather/model/weather_condition.hpp"

namespace avela::skin_piccolo {

// The icon for `condition` in a `size` x `size` square around `center`; at night (`daytime` false) a moon instead of
// the sun. `background` is what it is drawn on (the moon's crescent is cut out with it).
void draw_weather_icon(Painter& painter, Point center, int size, WeatherCondition condition, bool daytime,
                       uint32_t background);

}  // namespace avela::skin_piccolo
