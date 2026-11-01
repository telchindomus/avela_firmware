// A radio station as the screens show it.
#pragma once

namespace avela {

// What a skin draws for a station (texts owned by the StationList).
struct Station {
    const char* m_name = "";
    const char* m_monogram = "";  // shown in the station tile until stations have logos; from the name
    const char* m_genre = "";
};

}  // namespace avela
