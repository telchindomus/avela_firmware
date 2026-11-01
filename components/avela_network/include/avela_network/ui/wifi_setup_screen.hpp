// The Wi-Fi setup screen (Info's child): its view and controller.
#pragma once

#include <utility>

#include "avela_network/model/network_model.hpp"
#include "avela_network/ui/wifi_setup_controller.hpp"
#include "avela_network/ui/wifi_setup_skin_concept.hpp"
#include "avela_network/ui/wifi_setup_view.hpp"
#include "avela_ui/screen_id.hpp"

namespace avela {

template <WifiSetupSkinConcept PageSkin>
struct WifiSetupScreen {
    static constexpr ScreenId id = ScreenId::wifi_setup;

    WifiSetupScreen(const NetworkModel& model, PageSkin skin) : m_view{model, std::move(skin)} {}

    WifiSetupView<PageSkin> m_view;
    WifiSetupController m_controller;
};

}  // namespace avela
