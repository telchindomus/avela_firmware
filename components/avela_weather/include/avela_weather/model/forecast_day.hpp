// One day of the forecast row (skin API). Platform-free.
#pragma once

#include "avela_weather/model/weather_condition.hpp"

namespace avela {

struct ForecastDay {
    int m_weekday = 0;  // 0 = Sunday
    WeatherCondition m_condition = WeatherCondition::clear;
    int m_high = 0;
    int m_low = 0;
};

}  // namespace avela
