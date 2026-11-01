// The time zone, as saved by the setup page's clock section (a Settings record, avela_settings): a POSIX TZ string
// (e.g. "CET-1CEST,M3.5.0,M10.5.0/3": the zone and its daylight-saving rule). Without a saved one, the menuconfig
// default applies (Avela clock).
#pragma once

#include <cstdint>

#include "avela_strings/fixed_string.hpp"

namespace avela {

struct ClockSettings {
    static constexpr const char* key = "clock";
    static constexpr uint16_t version = 1;
    static constexpr std::size_t time_zone_capacity = 63 + 1;

    FixedString<time_zone_capacity> m_time_zone;
};

}  // namespace avela
