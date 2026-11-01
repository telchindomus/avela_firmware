// The station list's page model: which station is selected (an index into RadioModel::m_stations). Written by the
// StationsController, read by the StationsView; built with the screen (on the station playing) and gone with it.
#pragma once

#include <algorithm>
#include <cstddef>

namespace avela {

struct StationsModel {
    std::size_t m_selected = 0;

    // The selection in a list of `count` stations (never empty): the list may have shrunk while the screen was open
    // (stations removed on the setup page), then it is the last one.
    [[nodiscard]] std::size_t selected(std::size_t count) const { return std::min(m_selected, count - 1); }
};

}  // namespace avela
