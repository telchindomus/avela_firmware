// The station list (the player's child): its page model (the selected station, starting on the one playing), view and
// controller.
#pragma once

#include <utility>

#include "avela_radio/model/radio_model.hpp"
#include "avela_radio/ui/stations_controller.hpp"
#include "avela_radio/ui/stations_model.hpp"
#include "avela_radio/ui/stations_skin_concept.hpp"
#include "avela_radio/ui/stations_view.hpp"
#include "avela_ui/screen_id.hpp"

namespace avela {

template <StationsSkinConcept PageSkin>
struct StationsScreen {
    static constexpr ScreenId id = ScreenId::stations;

    StationsScreen(const RadioModel& model, PageSkin skin)
        : m_model{.m_selected = model.m_station_index}, m_view{model, m_model, std::move(skin)},
          m_controller{model, m_model}
    {}

    StationsModel m_model;
    StationsView<PageSkin> m_view;
    StationsController m_controller;
};

}  // namespace avela
