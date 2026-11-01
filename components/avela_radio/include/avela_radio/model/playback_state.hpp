// Where the radio is with the station it was asked to play (RadioModel, reported by the radio service).
#pragma once

#include <cstdint>

namespace avela {

enum class PlaybackState : uint8_t {
    stopped,     // paused, or not on the home network yet
    connecting,  // asking the station, or filling the buffer before the first sound
    playing,
    failed,  // the station didn't answer with an MP3 stream; tried again every few seconds
};

}  // namespace avela
