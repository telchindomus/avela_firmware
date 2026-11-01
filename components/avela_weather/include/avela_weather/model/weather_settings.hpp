// Where the weather is for and in which units, as saved by the setup page's weather section (a Settings record,
// avela_settings). The place is what the user typed; the weather service looks it up once (Open-Meteo's geocoder)
// and stores its coordinates here, so later updates go straight to the forecast.
#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "avela_strings/fixed_string.hpp"
#include "avela_weather/model/measurement_system.hpp"
#include "avela_weather/model/weather_report.hpp"

namespace avela {

struct WeatherSettings {
    static constexpr const char* key = "weather";
    static constexpr uint16_t version = 1;
    static constexpr std::size_t located_name_capacity = 64 + 1;
    static constexpr int32_t micro_degrees_per_degree = 1'000'000;

    FixedString<WeatherReport::place_capacity> m_place;  // empty: no weather
    MeasurementSystem m_units = MeasurementSystem::metric;
    bool m_located = false;  // the coordinates below belong to m_place
    int32_t m_latitude_micro_degrees = 0;
    int32_t m_longitude_micro_degrees = 0;
    FixedString<located_name_capacity> m_located_name;  // the geocoder's name for it, e.g. "Warsaw, Poland"
};

// Copies the coordinates found for `looked_up`'s place into `saved`, if `saved` still holds that place and isn't
// located yet (the setup page may have saved another place while it was looked up); false, `saved` untouched,
// otherwise.
inline bool take_location(WeatherSettings& saved, const WeatherSettings& looked_up)
{
    if (saved.m_located || !looked_up.m_located || saved.m_place.view() != looked_up.m_place.view()) {
        return false;
    }
    saved.m_located = true;
    saved.m_latitude_micro_degrees = looked_up.m_latitude_micro_degrees;
    saved.m_longitude_micro_degrees = looked_up.m_longitude_micro_degrees;
    saved.m_located_name = looked_up.m_located_name;
    return true;
}

// The settings after a save of `place` and `units` (valid; a field not given keeps `stored`'s value): an unchanged
// place keeps its coordinates, another one is looked up again.
inline WeatherSettings merged_settings(const WeatherSettings& stored, std::optional<std::string_view> place,
                                       std::optional<std::string_view> units)
{
    WeatherSettings settings{};
    settings.m_place.append(place.value_or(stored.m_place.view()));
    settings.m_units = stored.m_units;
    if (units) {
        settings.m_units = *units == "imperial" ? MeasurementSystem::imperial : MeasurementSystem::metric;
    }
    if (stored.m_located && stored.m_place.view() == settings.m_place.view()) {
        settings.m_located = true;
        settings.m_latitude_micro_degrees = stored.m_latitude_micro_degrees;
        settings.m_longitude_micro_degrees = stored.m_longitude_micro_degrees;
        settings.m_located_name = stored.m_located_name;
    }
    return settings;
}

// Makes `report` belong to `settings`: a report for another place or in other units starts again, empty, with their
// place and units (its revision kept, so the next publish still counts as a change); false if it already belonged.
inline bool match_report(WeatherReport& report, const WeatherSettings& settings)
{
    if (report.m_place.view() == settings.m_place.view() && report.m_units == settings.m_units) {
        return false;
    }
    const uint32_t revision = report.m_revision;
    report = WeatherReport{};  // old values belong to another place or other units
    report.m_revision = revision;
    report.m_place.append(settings.m_place.view());
    report.m_units = settings.m_units;
    return true;
}

}  // namespace avela
