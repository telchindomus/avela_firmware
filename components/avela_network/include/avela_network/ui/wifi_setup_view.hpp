// Shown while the setup network is up (opened from the Info screen): how to reach the setup page. Hands the saved
// network, the setup network (menuconfig: Avela network) with its QR join code and the page's address to its skin.
#pragma once

#include <string_view>
#include <utility>

#include "avela_network/model/network_model.hpp"
#include "avela_network/ui/wifi_setup_skin_concept.hpp"
#include "sdkconfig.h"

namespace avela {

template <WifiSetupSkinConcept PageSkin>
class WifiSetupView {
public:
    WifiSetupView(const NetworkModel& model, PageSkin skin) : m_model_(model), m_skin_(std::move(skin)) {}

    void draw()
    {
        refresh();
        m_skin_.draw();
    }

    void refresh()
    {
        m_skin_.set_saved_network(m_model_.m_status, m_model_.m_ssid.view());
        m_skin_.set_setup_network(CONFIG_AVELA_SETUP_NETWORK_NAME, CONFIG_AVELA_SETUP_NETWORK_PASSWORD, join_code);
        m_skin_.set_setup_page(m_model_.m_setup_network_active, m_model_.m_setup_address.view());
    }

private:
    // Standard Wi-Fi QR code text (phones join the network when they scan it).
    static constexpr std::string_view join_code =
        "WIFI:T:WPA;S:" CONFIG_AVELA_SETUP_NETWORK_NAME ";P:" CONFIG_AVELA_SETUP_NETWORK_PASSWORD ";;";

    const NetworkModel& m_model_;
    PageSkin m_skin_;
};

}  // namespace avela
