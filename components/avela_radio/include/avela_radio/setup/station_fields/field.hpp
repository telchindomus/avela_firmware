// A Stations field: a slot and a part ("12.url" -> {12, url}).
#pragma once

#include <cstdint>

#include "avela_radio/setup/station_fields/part.hpp"

namespace avela::station_fields {

struct Field {
    uint8_t m_slot;  // 1..StationRecord::slot_count
    Part m_part;

    bool operator==(const Field&) const = default;
};

}  // namespace avela::station_fields
