// The tiles (the screen selector): hands the screens they offer and the chosen one to its skin.
#pragma once

#include <span>
#include <utility>

#include "avela_ui/screen_id.hpp"
#include "avela_ui/tiles/tiles_model.hpp"
#include "avela_ui/tiles/tiles_skin_concept.hpp"
#include "avela_ui/view_concept.hpp"

namespace avela {

template <TilesSkinConcept PageSkin>
class TilesView {
public:
    // screens: what the tiles offer, in order (static storage)
    TilesView(std::span<const ScreenId> screens, const TilesModel& page, PageSkin skin)
        : m_screens_(screens), m_page_(page), m_skin_(std::move(skin))
    {}

    void draw()
    {
        refresh();
        m_skin_.draw();
    }

    void refresh() { m_skin_.set_tiles(m_screens_, m_page_.m_selected); }

private:
    std::span<const ScreenId> m_screens_;
    const TilesModel& m_page_;
    PageSkin m_skin_;
};

}  // namespace avela
