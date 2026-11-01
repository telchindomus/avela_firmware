// Station list (the player's child): Increment / Decrement = move the selection (wrapping around), Ok = play the
// selected station and close the list (back to the player).
#pragma once

#include <cstddef>

#include "avela_messaging/messages/player_request.hpp"
#include "avela_radio/model/radio_model.hpp"
#include "avela_radio/ui/stations_model.hpp"
#include "avela_ui/command/command.hpp"
#include "avela_ui/controller_concept.hpp"
#include "avela_ui/response.hpp"

namespace avela {

class StationsController {
public:
    // page: the selection
    StationsController(const RadioModel& model, StationsModel& page) : m_model_(model), m_page_(page) {}

    Response handle(IncrementCommand increment)
    {
        move_selection(increment.m_steps);
        return {};
    }

    Response handle(DecrementCommand decrement)
    {
        move_selection(-decrement.m_steps);
        return {};
    }

    [[nodiscard]] Response handle(OkCommand /*ok*/) const
    {
        return {.m_close = true,
                .m_request = PlayerRequest{.m_station_slot =
                                               m_model_.m_stations.slot(m_page_.selected(m_model_.m_stations.size())),
                                           .m_playing = true}};
    }

private:
    void move_selection(int steps)
    {
        const std::size_t size = m_model_.m_stations.size();
        const int count = static_cast<int>(size);
        const int selected = static_cast<int>(m_page_.selected(size));
        m_page_.m_selected = static_cast<std::size_t>(((selected + steps) % count + count) % count);
    }

    const RadioModel& m_model_;
    StationsModel& m_page_;
};

static_assert(ControllerConcept<StationsController>);

}  // namespace avela
