// A station as stored: one Settings record per station slot (1..StationRecord::slot_count, keys "station1"..), written
// by the setup page's and the console's Stations section, read by the radio. An empty address = a free slot. Only the
// names and genres are kept in RAM (StationList); the address is read from the record when the station is tuned.
#pragma once

#include <cstddef>
#include <cstdint>

#include "avela_strings/fixed_string.hpp"

namespace avela {

struct StationRecord {
    static constexpr const char* key = "station";
    static constexpr uint16_t version = 1;
    static constexpr std::size_t slot_count = 16;
    static constexpr std::size_t name_capacity = 32 + 1;
    static constexpr std::size_t genre_capacity = 32 + 1;
    static constexpr std::size_t url_capacity = 200 + 1;

    FixedString<name_capacity> m_name;
    FixedString<genre_capacity> m_genre;
    FixedString<url_capacity> m_url;
};

}  // namespace avela
