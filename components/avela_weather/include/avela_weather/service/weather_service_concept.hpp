// What a weather service provides: it fulfils the model's update request and writes the report (update(), on the main
// loop's task, every frame). Implementation: OpenMeteoWeatherService.
#pragma once

#include <concepts>

#include "avela_weather/model/weather_model.hpp"

namespace avela {

template <typename T>
concept WeatherServiceConcept = requires(T& service, WeatherModel& model) {
    { service.init() } -> std::same_as<void>;  // once, at startup
    { service.update(model) } -> std::same_as<void>;
};

}  // namespace avela
