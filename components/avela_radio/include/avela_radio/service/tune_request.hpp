// The main loop to the stream task (StreamReceiver): the station to play; an empty URL = stop.
#pragma once

#include <cstddef>

#include "avela_strings/fixed_string.hpp"

namespace avela {

struct TuneRequest {
    static constexpr std::size_t url_capacity = 256;
    FixedString<url_capacity> m_url;
};

}  // namespace avela
