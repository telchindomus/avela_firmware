// The picture of what is playing (station or album). Until stations have pictures it is the station's monogram, which
// the skin draws in its tile.
#pragma once

#include <string_view>

namespace avela {

struct Thumbnail {
    std::string_view m_monogram;
};

}  // namespace avela
