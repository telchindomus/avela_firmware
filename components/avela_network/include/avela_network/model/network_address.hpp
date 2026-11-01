// How long an IPv4 address is as text, for whoever keeps one (NetworkModel, the skins). A header of its own: the skin
// API headers and the skins stay platform-free (NetworkModel pulls in FreeRTOS for its locked list).
#pragma once

#include <cstddef>

#include "avela_strings/fixed_string.hpp"

namespace avela {

inline constexpr std::size_t network_address_capacity = capacity_for("255.255.255.255");

}  // namespace avela
