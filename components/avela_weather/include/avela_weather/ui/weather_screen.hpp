// Weather: its view and controller.
#pragma once

#include <utility>

#include "avela_network/model/network_model.hpp"
#include "avela_ui/screen_id.hpp"
#include "avela_weather/model/weather_model.hpp"
#include "avela_weather/ui/weather_controller.hpp"
#include "avela_weather/ui/weather_skin_concept.hpp"
#include "avela_weather/ui/weather_view.hpp"

namespace avela {

template <WeatherSkinConcept PageSkin>
struct WeatherScreen {
    static constexpr ScreenId id = ScreenId::weather;

    WeatherScreen(const WeatherModel& model, const NetworkModel& network, PageSkin skin)
        : m_view{model, network, std::move(skin)}
    {}

    WeatherView<PageSkin> m_view;
    WeatherController m_controller;
};

}  // namespace avela
