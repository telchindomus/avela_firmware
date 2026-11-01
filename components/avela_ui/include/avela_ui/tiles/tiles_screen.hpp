// The tiles: the screen selector, the root of the screen tree.
#pragma once

#include <cstddef>
#include <span>
#include <utility>

#include "avela_ui/screen_id.hpp"
#include "avela_ui/tiles/tiles_controller.hpp"
#include "avela_ui/tiles/tiles_model.hpp"
#include "avela_ui/tiles/tiles_skin_concept.hpp"
#include "avela_ui/tiles/tiles_view.hpp"

namespace avela {

template <TilesSkinConcept PageSkin>
struct TilesScreen {
    static constexpr ScreenId id = ScreenId::tiles;

    // screens: what the tiles offer, in order (static storage); from: the screen closed to come back to the tiles,
    // which they open on
    TilesScreen(std::span<const ScreenId> screens, ScreenId from, PageSkin skin)
        : m_model{.m_selected = position_of(screens, from)}, m_view{screens, m_model, std::move(skin)},
          m_controller{screens, m_model}
    {}

    TilesModel m_model;
    TilesView<PageSkin> m_view;
    TilesController m_controller;

private:
    static std::size_t position_of(std::span<const ScreenId> screens, ScreenId screen)
    {
        for (std::size_t position = 0; position < screens.size(); ++position) {
            if (screens[position] == screen) {
                return position;
            }
        }
        return 0;
    }
};

}  // namespace avela
