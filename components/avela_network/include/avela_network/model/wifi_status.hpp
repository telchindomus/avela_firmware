// The state of the saved home network, as NetworkModel reports it. A header of its own because the skins take it
// (skin API: wifi_setup_view, info_view, status_tray_skin), and the skin API headers stay platform-free.
#pragma once

#include <cstdint>

namespace avela {

enum class WifiStatus : uint8_t {
    not_configured,  // no home network saved
    connecting,      // joining the saved network, retrying until it answers
    connected,       // joined, with an address
};

}  // namespace avela
