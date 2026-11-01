// The radio's controls, the same on every screen whose knob has no other use there (the header shows their effect on
// every screen): Increment / Decrement = volume (volume_step per step), Ok = play/stop. A controller takes them by
// deriving from PlaybackControls (`using PlaybackControls::handle;` next to its own handle() overloads).
#pragma once

#include "avela_messaging/messages/player_request.hpp"
#include "avela_ui/command/command.hpp"
#include "avela_ui/response.hpp"

namespace avela {

class PlaybackControls {
public:
    static constexpr int volume_step = 2;  // per step

    static Response handle(IncrementCommand increment) { return volume_by(increment.m_steps); }

    static Response handle(DecrementCommand decrement) { return volume_by(-decrement.m_steps); }

    static Response handle(OkCommand /*ok*/) { return {.m_request = PlayerRequest{.m_toggle_playing = true}}; }

private:
    static Response volume_by(int steps)
    {
        return {.m_request = PlayerRequest{.m_volume_change = steps * volume_step}};
    }
};

}  // namespace avela
