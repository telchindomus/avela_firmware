// The tiles' page model: which tile is chosen. Written by the TilesController, read by the TilesView; built with the
// screen and gone with it.
#pragma once

#include <cstddef>

namespace avela {

struct TilesModel {
    std::size_t m_selected = 0;  // in the screens the tiles offer; starts on the branch they were opened from
};

}  // namespace avela
