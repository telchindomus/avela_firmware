// The Silk Taupe skin's tiles (the screen selector, the root of the screen tree): a centred row of tiles, one per
// screen main offers (in its order; the screens of components that are switched off aren't there), each with its icon
// (the player: the Avela symbol; weather: sun and cloud; info: an "i") and label, the chosen one outlined in the accent
// colour, the hint at the bottom. Fulfils TilesSkinConcept (avela_ui/tiles/tiles_skin_concept.hpp).
#pragma once

#include <cstddef>
#include <span>

#include "avela_skin_piccolo_silk_taupe/surface.hpp"
#include "avela_ui/screen_id.hpp"
#include "avela_ui/tiles/tiles_skin_concept.hpp"

namespace avela::skin_piccolo_silk_taupe {

class TilesSkin {
public:
    explicit TilesSkin(const Surface& surface) : m_surface_(surface) {}

    void set_tiles(std::span<const ScreenId> screens, std::size_t selected);
    void draw();

private:
    void render() const;
    void draw_tile(Painter& painter, ScreenId shown, Rect area, bool selected) const;

    Surface m_surface_;
    std::span<const ScreenId> m_screens_;
    std::size_t m_selected_ = 0;
    bool m_drawn_ = false;
};

static_assert(TilesSkinConcept<TilesSkin>);

}  // namespace avela::skin_piccolo_silk_taupe
