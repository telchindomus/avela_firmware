// Station list: hands the stations, the selected one and the one playing to its skin.
#pragma once

#include <utility>

#include "avela_radio/model/radio_model.hpp"
#include "avela_radio/ui/stations_model.hpp"
#include "avela_radio/ui/stations_skin_concept.hpp"

namespace avela {

template <StationsSkinConcept PageSkin>
class StationsView {
public:
    StationsView(const RadioModel& model, const StationsModel& page, PageSkin skin)
        : m_model_(model), m_page_(page), m_skin_(std::move(skin))
    {}

    void draw()
    {
        refresh();
        m_skin_.draw();
    }

    void refresh()
    {
        const auto stations = m_model_.m_stations.stations();
        m_skin_.set_selection(stations, m_page_.selected(stations.size()), m_model_.m_station_index);
    }

private:
    const RadioModel& m_model_;
    const StationsModel& m_page_;
    PageSkin m_skin_;
};

}  // namespace avela
