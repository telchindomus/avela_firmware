// Board information (Info tile): the playback controls (volume, play/stop), Options = the Wi-Fi setup screen (which
// asks for the setup network), Menu = the firmware update screen; both are its children in the screen tree.
#pragma once

#include "avela_ui/command/command.hpp"
#include "avela_ui/controller_concept.hpp"
#include "avela_ui/playback_controls.hpp"
#include "avela_ui/response.hpp"

namespace avela {

class InfoController : public PlaybackControls {
public:
    using PlaybackControls::handle;

    static Response handle(OptionsCommand /*options*/) { return {.m_show = ScreenId::wifi_setup}; }

    static Response handle(MenuCommand /*menu*/) { return {.m_show = ScreenId::firmware_update}; }
};

static_assert(ControllerConcept<InfoController>);

}  // namespace avela
