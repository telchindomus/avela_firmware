// Weather: hands the weather model's report and the radio's host name (NetworkModel) to its skin.
#pragma once

#include <utility>

#include "avela_network/model/network_model.hpp"
#include "avela_weather/model/weather_model.hpp"
#include "avela_weather/ui/weather_skin_concept.hpp"

namespace avela {

template <WeatherSkinConcept PageSkin>
class WeatherView {
public:
    WeatherView(const WeatherModel& model, const NetworkModel& network, PageSkin skin)
        : m_model_(model), m_network_(network), m_skin_(std::move(skin))
    {}

    void draw()
    {
        refresh();
        m_skin_.draw();
    }

    void refresh()
    {
        m_skin_.set_weather(m_model_.m_report);
        m_skin_.set_host_name(m_network_.m_host_name.view());
    }

private:
    const WeatherModel& m_model_;
    const NetworkModel& m_network_;
    PageSkin m_skin_;
};

}  // namespace avela
