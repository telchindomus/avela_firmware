// The weather as the screen sees it: the report, written by the weather service.
#pragma once

#include "avela_weather/model/weather_report.hpp"

namespace avela {

struct WeatherModel {
    WeatherReport m_report;
};

}  // namespace avela
