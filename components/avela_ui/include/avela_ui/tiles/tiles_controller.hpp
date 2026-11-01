// The tiles (the screen selector, the root of the screen tree): Increment / Decrement = choose (no wrap), Ok = open
// the chosen screen.
#pragma once

#include <algorithm>
#include <cstddef>
#include <span>

#include "avela_ui/command/command.hpp"
#include "avela_ui/controller_concept.hpp"
#include "avela_ui/response.hpp"
#include "avela_ui/screen_id.hpp"
#include "avela_ui/tiles/tiles_model.hpp"

namespace avela {

class TilesController {
public:
    // screens: what the tiles offer, in order (static storage); page: the chosen tile
    TilesController(std::span<const ScreenId> screens, TilesModel& page) : m_screens_(screens), m_page_(page) {}

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

    [[nodiscard]] Response handle(OkCommand /*ok*/) const { return {.m_show = m_screens_[m_page_.m_selected]}; }

private:
    void move_selection(int steps)
    {
        const int last = static_cast<int>(m_screens_.size()) - 1;
        m_page_.m_selected =
            static_cast<std::size_t>(std::clamp(static_cast<int>(m_page_.m_selected) + steps, 0, last));
    }

    std::span<const ScreenId> m_screens_;
    TilesModel& m_page_;
};

static_assert(ControllerConcept<TilesController>);

}  // namespace avela
