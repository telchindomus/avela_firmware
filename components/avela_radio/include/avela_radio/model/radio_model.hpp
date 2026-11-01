// The radio's state as the screens see it. Two kinds of fields:
// - requested: what the user asked for (station, volume, play/pause), written only by carry_out(PlayerRequest), whoever
//   made the request (the knob's controllers, the setup page, the console);
// - reported: written by the radio service, which follows the requested fields (playback state, stream title, format,
//   levels).
// The station list is loaded from the settings by RadioStorage. All on the main loop. The model has no behaviour of
// its own; where the reported values come from is the service's business.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "avela_radio/model/playback_state.hpp"
#include "avela_radio/model/station.hpp"
#include "avela_radio/model/station_list.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela {

struct RadioModel {
    static constexpr int volume_min = 0;
    static constexpr int volume_max = 100;
    static constexpr int volume_default = 62;
    static constexpr std::size_t spectrum_band_count = 32;
    static constexpr uint8_t spectrum_level_max = 255;
    static constexpr std::size_t stream_title_capacity = 128;  // "Artist – Title" from the stream's metadata
    static constexpr std::size_t stream_format_capacity = 32;  // "128 kbit/s MP3"

    StationList m_stations;

    // requested
    std::size_t m_station_index = 0;  // in m_stations
    int m_volume = volume_default;    // volume_min..volume_max
    bool m_playing = true;

    // reported
    PlaybackState m_playback_state = PlaybackState::stopped;
    FixedString<stream_title_capacity> m_stream_title;
    FixedString<stream_format_capacity> m_stream_format;
    // signal level per frequency band, low -> high, 0..spectrum_level_max; how it is shown is the skin's
    std::array<uint8_t, spectrum_band_count> m_spectrum{};

    [[nodiscard]] const Station& station() const { return m_stations.stations()[m_station_index]; }
};

}  // namespace avela
