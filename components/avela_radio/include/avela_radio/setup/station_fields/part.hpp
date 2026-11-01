// Which part of a station a Stations field is: "<slot>.name", "<slot>.genre" or "<slot>.url".
#pragma once

#include <cstdint>

namespace avela::station_fields {

enum class Part : uint8_t { name, genre, url };

}  // namespace avela::station_fields
