// The Silk Taupe skin's Wi-Fi setup screen: SETUP badge, headline, the saved network; while the setup network starts
// only "Please wait…", once it is up its name and password, the page's address and the QR code that joins it. Redraws
// whole when anything changes. Fulfils WifiSetupSkin (avela_network).
#pragma once

#include <string_view>

#include "avela_network/model/network_address.hpp"
#include "avela_network/model/wifi_credentials.hpp"
#include "avela_network/model/wifi_status.hpp"
#include "avela_network/ui/wifi_setup_skin_concept.hpp"
#include "avela_skin_piccolo_silk_taupe/surface.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela::skin_piccolo_silk_taupe {

class WifiSetupSkin {
public:
    explicit WifiSetupSkin(const Surface& surface) : m_surface_(surface) {}

    void set_saved_network(WifiStatus status, std::string_view ssid);
    void set_setup_network(std::string_view name, std::string_view password, std::string_view join_code);
    void set_setup_page(bool active, std::string_view address);
    void draw();

private:
    // "WIFI:T:WPA;S:<name>;P:<password>;;"
    static constexpr std::size_t join_code_capacity =
        capacity_for("WIFI:T:WPA;S:;P:;;") + WifiCredentials::ssid_capacity + WifiCredentials::password_capacity;

    void render() const;

    Surface m_surface_;
    bool m_shown_ = false;
    WifiStatus m_saved_status_ = WifiStatus::not_configured;
    FixedString<WifiCredentials::ssid_capacity> m_saved_ssid_;
    FixedString<WifiCredentials::ssid_capacity> m_setup_name_;
    FixedString<WifiCredentials::password_capacity> m_setup_password_;
    FixedString<join_code_capacity> m_join_code_;
    bool m_setup_active_ = false;
    FixedString<network_address_capacity> m_page_address_;
};

}  // namespace avela::skin_piccolo_silk_taupe

namespace avela {
static_assert(WifiSetupSkinConcept<skin_piccolo_silk_taupe::WifiSetupSkin>);
}  // namespace avela
