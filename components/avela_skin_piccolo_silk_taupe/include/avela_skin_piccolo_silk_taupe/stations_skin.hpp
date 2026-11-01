// The Silk Taupe skin's station list: the selected station in a highlighted row with its position ("03 / 05"), its
// neighbours above and below, a hint line at the bottom. A play triangle marks the station playing: in front of a
// neighbour's name, in front of the selected row's position. Fulfils
// StationsSkin (avela_radio).
#pragma once

#include <cstddef>
#include <span>

#include "avela_radio/model/station.hpp"
#include "avela_radio/ui/stations_skin_concept.hpp"
#include "avela_skin_piccolo_silk_taupe/surface.hpp"

namespace avela::skin_piccolo_silk_taupe {

class StationsSkin {
public:
    explicit StationsSkin(const Surface& surface) : m_surface_(surface) {}

    void set_selection(std::span<const Station> stations, std::size_t selected, std::size_t playing);
    void draw();

private:
    void draw_list(Painter& painter) const;
    static void draw_playing_marker(Painter& painter, int center_x, int center_y);
    [[nodiscard]] std::size_t wrapped(int offset) const;  // the station `offset` rows from the selected one

    Surface m_surface_;
    bool m_shown_ = false;
    std::span<const Station> m_stations_;
    std::size_t m_selected_ = 0;
    std::size_t m_playing_ = 0;
};

}  // namespace avela::skin_piccolo_silk_taupe

namespace avela {
static_assert(StationsSkinConcept<skin_piccolo_silk_taupe::StationsSkin>);
}  // namespace avela
