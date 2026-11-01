// The stream task to the main loop (a latest-value queue): its session, how the connection goes and the stream's
// title.
#pragma once

#include <cstdint>

#include "avela_radio/model/radio_model.hpp"
#include "avela_radio/service/reception_status.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela {

struct ReceptionReport {
    uint32_t m_session = 0;
    ReceptionStatus m_status = ReceptionStatus::idle;
    FixedString<RadioModel::stream_title_capacity> m_title;
};

}  // namespace avela
