// Now playing: the playback controls (volume, play/stop), Menu = the station list (its child in the screen tree).
#pragma once

#include "avela_ui/command/command.hpp"
#include "avela_ui/controller_concept.hpp"
#include "avela_ui/playback_controls.hpp"
#include "avela_ui/response.hpp"

namespace avela {

class PlayerController : public PlaybackControls {
public:
    using PlaybackControls::handle;

    static Response handle(MenuCommand /*menu*/) { return {.m_show = ScreenId::stations}; }
};

static_assert(ControllerConcept<PlayerController>);

}  // namespace avela
