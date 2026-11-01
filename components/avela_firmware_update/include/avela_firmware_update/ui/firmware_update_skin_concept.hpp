// What the firmware update screen shows, as its skin must take it: the running version, where to
// upload the new firmware (the page's address by host name and, while connected, by IP address) and the update's
// state with its progress, as concrete values. Once shown (draw()), each setter redraws at once if a value differs from
// what it shows; the screen calls every setter each frame.
// Skin authors: components/avela_firmware_update/docs/developer/SKIN.md describes every setter (values, ranges,
// controls).
#pragma once

#include <concepts>
#include <string_view>

#include "avela_firmware_update/model/firmware_update_state.hpp"

namespace avela {

template <typename T>
concept FirmwareUpdateSkinConcept = requires(T& skin, std::string_view text, FirmwareUpdateState state, int percent) {
    { skin.set_running_version(text) } -> std::same_as<void>;
    { skin.set_update_page(text, text) } -> std::same_as<void>;         // by host name, by IP address (empty if none)
    { skin.set_progress(state, percent, text) } -> std::same_as<void>;  // percent 0..100, why it failed
    { skin.draw() } -> std::same_as<void>;
};

}  // namespace avela
