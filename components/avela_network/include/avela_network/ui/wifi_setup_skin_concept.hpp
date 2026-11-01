// What the Wi-Fi setup screen shows, as its skin must take it: the saved network, the setup network
// (its name, password and the QR code text that joins it, whether it is up yet) and the setup page's address, as
// concrete values. Once shown (draw()), each setter redraws at once if a value differs from what it shows; the screen
// calls every setter each frame.
// Skin authors: components/avela_network/docs/developer/SKIN.md describes every setter (values, ranges, controls).
#pragma once

#include <concepts>
#include <string_view>

#include "avela_network/model/wifi_status.hpp"

namespace avela {

template <typename T>
concept WifiSetupSkinConcept = requires(T& skin, WifiStatus status, std::string_view text, bool active) {
    { skin.set_saved_network(status, text) } -> std::same_as<void>;      // status, ssid
    { skin.set_setup_network(text, text, text) } -> std::same_as<void>;  // name, password, join code
    { skin.set_setup_page(active, text) } -> std::same_as<void>;         // setup network up, page address
    { skin.draw() } -> std::same_as<void>;
};

}  // namespace avela
