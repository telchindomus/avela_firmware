// The weather as the weather screen shows it (skin API): the state of the service and the values of the last successful
// update, whole numbers in the chosen units. One value the skin takes as a whole (WeatherSkin::set_weather): it
// changes every few minutes at most, and m_revision tells a skin whether anything changed. Platform-free.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "avela_strings/fixed_string.hpp"
#include "avela_weather/model/forecast_day.hpp"
#include "avela_weather/model/measurement_system.hpp"
#include "avela_weather/model/weather_condition.hpp"
#include "avela_weather/model/weather_status.hpp"

namespace avela {

struct WeatherReport {
    static constexpr std::size_t place_capacity = 40 + 1;
    static constexpr std::size_t forecast_days = 4;  // the days after today

    WeatherStatus m_status = WeatherStatus::not_configured;
    FixedString<place_capacity> m_place;  // as typed on the setup page
    MeasurementSystem m_units = MeasurementSystem::metric;
    bool m_has_values = false;  // the values below come from a successful update (maybe an older one)
    int m_temperature = 0;
    int m_feels_like = 0;
    WeatherCondition m_condition = WeatherCondition::clear;
    bool m_daytime = true;  // night: the skins may show a moon instead of a sun
    int m_humidity_percent = 0;
    int m_wind_speed = 0;  // km/h or mph
    int m_high = 0;        // today
    int m_low = 0;
    std::array<ForecastDay, forecast_days> m_forecast{};
    uint32_t m_revision = 0;  // raised whenever anything above changes
};

}  // namespace avela
