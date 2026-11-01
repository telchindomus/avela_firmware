// Reading Open-Meteo's answers (forecast and geocoding API, JSON) without a heap: the functions look at the received
// text in place and return skins into it. Only what Avela needs: values by key in an object, numbers and strings in
// arrays. Strings come back as they are in the JSON (escapes not undone; Open-Meteo sends plain UTF-8). Platform-free.
#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

#include "avela_weather/model/weather_condition.hpp"

namespace avela::open_meteo {

// The raw value of `key` in the JSON object `object` (which starts with '{'), looking only at its own keys, not at
// those of nested objects: a number, "string" (with its quotes), [array] or {object}; empty if there is none.
std::string_view value_of(std::string_view object, std::string_view key);

std::optional<double> number(std::string_view value);
// The text between a string value's quotes.
std::optional<std::string_view> string(std::string_view value);
// The elements of an array value ([...]), each as a raw value; returns how many it put into `elements`.
std::size_t elements(std::string_view array, std::span<std::string_view> elements);

// WMO weather interpretation codes (Open-Meteo's weather_code) in Avela's groups.
WeatherCondition condition_of(int weather_code);

// The weekday (0 = Sunday) of an ISO date "YYYY-MM-DD"; nothing if it isn't one.
std::optional<int> weekday_of(std::string_view date);

}  // namespace avela::open_meteo
