// The player task to the main loop (a latest-value queue): whether sound of its session comes out, the format and the
// spectrum.
#pragma once

#include <cstdint>

#include "avela_radio/audio/spectrum_analyzer.hpp"
#include "avela_radio/model/radio_model.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela {

struct PlaybackReport {
    uint32_t m_session = 0;
    bool m_sounding = false;                                   // frames of this session are playing
    FixedString<RadioModel::stream_format_capacity> m_format;  // "128 kbit/s MP3"
    SpectrumAnalyzer::Levels m_spectrum{};
};

}  // namespace avela
