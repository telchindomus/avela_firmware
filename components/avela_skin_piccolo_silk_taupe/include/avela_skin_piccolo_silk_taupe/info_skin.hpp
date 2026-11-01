// The Silk Taupe skin's info screen: INFO badge, label/value rows (Network, Address, Signal, Memory, Firmware; "–"
// where nothing applies), the free internal RAM as a ring with its percentage, and the hint. Shows uptime in hours and
// minutes, so it redraws at most once a minute for it. Fulfils InfoSkin (avela_system_info).
#pragma once

#include <chrono>
#include <cstddef>
#include <string_view>

#include "avela_network/model/host_name.hpp"
#include "avela_network/model/network_address.hpp"
#include "avela_network/model/wifi_credentials.hpp"
#include "avela_network/model/wifi_status.hpp"
#include "avela_skin_piccolo_silk_taupe/surface.hpp"
#include "avela_skin_piccolo_silk_taupe/widgets.hpp"
#include "avela_system_info/ui/info_skin_concept.hpp"

namespace avela::skin_piccolo_silk_taupe {

class InfoSkin {
public:
    explicit InfoSkin(const Surface& surface) : m_surface_(surface) {}

    void set_network(WifiStatus status, std::string_view ssid, std::string_view address, std::string_view host_name,
                     int signal_dbm);
    void set_memory(std::size_t free_kilobytes, std::size_t minimum_free_kilobytes, std::size_t total_kilobytes);
    void set_firmware(std::string_view version, std::chrono::seconds uptime);
    void draw();

private:
    void render_details() const;
    void draw_details(Painter& painter) const;

    Surface m_surface_;
    bool m_shown_ = false;
    WifiStatus m_status_ = WifiStatus::not_configured;
    FixedString<WifiCredentials::ssid_capacity> m_ssid_;
    FixedString<network_address_capacity> m_address_;
    FixedString<HostName::capacity> m_host_name_;
    int m_signal_dbm_ = 0;
    std::size_t m_free_kilobytes_ = 0;
    std::size_t m_minimum_free_kilobytes_ = 0;
    std::size_t m_total_kilobytes_ = 0;
    LabelText m_version_;
    std::chrono::minutes m_uptime_{};
};

}  // namespace avela::skin_piccolo_silk_taupe

namespace avela {
static_assert(InfoSkinConcept<skin_piccolo_silk_taupe::InfoSkin>);
}  // namespace avela
