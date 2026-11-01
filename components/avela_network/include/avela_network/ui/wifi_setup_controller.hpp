// The Wi-Fi setup screen (Info's child): the playback controls (volume, play/stop); shown, it asks for the setup
// network; left, it closes it again. Saving a network on the setup page closes it too (the network service; the Ui
// then shows Info).
#pragma once

#include "avela_messaging/messages/request.hpp"
#include "avela_messaging/messages/setup_network_request.hpp"
#include "avela_ui/command/command.hpp"
#include "avela_ui/controller_concept.hpp"
#include "avela_ui/playback_controls.hpp"
#include "avela_ui/response.hpp"

namespace avela {

class WifiSetupController : public PlaybackControls {
public:
    static Request shown() { return SetupNetworkRequest{.m_open = true}; }

    static Request left() { return SetupNetworkRequest{.m_open = false}; }
};

static_assert(ControllerConcept<WifiSetupController>);

}  // namespace avela
