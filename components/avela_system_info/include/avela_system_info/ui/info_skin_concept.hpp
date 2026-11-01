// What the info screen shows, as its skin must take it: the network, internal RAM and firmware as
// concrete values, no models; the skin formats them. Once shown (draw()), each setter redraws its part at once if a
// value differs from what it shows; the screen calls every setter each frame.
// Skin authors: components/avela_system_info/docs/developer/SKIN.md describes every setter (values, ranges, controls).
#pragma once

#include <chrono>
#include <concepts>
#include <cstddef>
#include <string_view>

#include "avela_network/model/wifi_status.hpp"

namespace avela {

template <typename T>
concept InfoSkinConcept = requires(T& skin, WifiStatus status, std::string_view text, int signal_dbm,
                                   std::size_t kilobytes, std::chrono::seconds uptime) {
    // status, ssid, address, host name (http://<host name>.local), signal
    { skin.set_network(status, text, text, text, signal_dbm) } -> std::same_as<void>;
    { skin.set_memory(kilobytes, kilobytes, kilobytes) } -> std::same_as<void>;  // free, lowest free, total
    { skin.set_firmware(text, uptime) } -> std::same_as<void>;                   // version, time since start
    { skin.draw() } -> std::same_as<void>;
};

}  // namespace avela
