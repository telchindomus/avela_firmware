// What a network service provides: it joins the saved home network (Settings: WifiCredentials), opens the setup
// network when the model requests it, and reports all of it into the NetworkModel.
// Implementation: EspWifiService (ESP-IDF). The app picks one at compile time.
#pragma once

#include <concepts>

#include "avela_network/model/network_model.hpp"

namespace avela {

template <typename T>
concept NetworkServiceConcept = requires(T& service, NetworkModel& model) {
    { service.init() } -> std::same_as<void>;         // once: start the network stack and Wi-Fi
    { service.update(model) } -> std::same_as<void>;  // every frame: follow settings and events, write the reports
};

}  // namespace avela
