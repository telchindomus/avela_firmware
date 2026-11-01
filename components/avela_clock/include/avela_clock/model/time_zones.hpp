// The time zones the setup page offers by name, each with its POSIX TZ string (the zone and its daylight-saving
// rule). Others can be set as a POSIX TZ string on the serial console. Platform-free.
#pragma once

#include <array>
#include <string_view>

namespace avela {

struct NamedTimeZone {
    std::string_view m_name;
    std::string_view m_posix;
};

inline constexpr std::array named_time_zones{
    NamedTimeZone{.m_name = "UTC", .m_posix = "UTC0"},
    NamedTimeZone{.m_name = "London, Dublin", .m_posix = "GMT0BST,M3.5.0/1,M10.5.0"},
    NamedTimeZone{.m_name = "Lisbon", .m_posix = "WET0WEST,M3.5.0/1,M10.5.0"},
    NamedTimeZone{.m_name = "Warsaw, Berlin, Paris, Rome, Madrid", .m_posix = "CET-1CEST,M3.5.0,M10.5.0/3"},
    NamedTimeZone{.m_name = "Kyiv, Helsinki, Athens, Bucharest", .m_posix = "EET-2EEST,M3.5.0/3,M10.5.0/4"},
    NamedTimeZone{.m_name = "Istanbul", .m_posix = "<+03>-3"},
    NamedTimeZone{.m_name = "Dubai", .m_posix = "<+04>-4"},
    NamedTimeZone{.m_name = "India", .m_posix = "IST-5:30"},
    NamedTimeZone{.m_name = "Singapore", .m_posix = "<+08>-8"},
    NamedTimeZone{.m_name = "China", .m_posix = "CST-8"},
    NamedTimeZone{.m_name = "Japan", .m_posix = "JST-9"},
    NamedTimeZone{.m_name = "Sydney, Melbourne", .m_posix = "AEST-10AEDT,M10.1.0,M4.1.0/3"},
    NamedTimeZone{.m_name = "New Zealand", .m_posix = "NZST-12NZDT,M9.5.0,M4.1.0/3"},
    NamedTimeZone{.m_name = "São Paulo", .m_posix = "<-03>3"},
    NamedTimeZone{.m_name = "New York (US Eastern)", .m_posix = "EST5EDT,M3.2.0,M11.1.0"},
    NamedTimeZone{.m_name = "Chicago (US Central)", .m_posix = "CST6CDT,M3.2.0,M11.1.0"},
    NamedTimeZone{.m_name = "Denver (US Mountain)", .m_posix = "MST7MDT,M3.2.0,M11.1.0"},
    NamedTimeZone{.m_name = "Phoenix", .m_posix = "MST7"},
    NamedTimeZone{.m_name = "Los Angeles (US Pacific)", .m_posix = "PST8PDT,M3.2.0,M11.1.0"},
};

}  // namespace avela
