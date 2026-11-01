// The network as the screens, the tray and the setup page see it, written by the network service. Which network to
// join comes through Settings (WifiCredentials), because the setup page that saves it runs on another task. Read on the
// main loop's task, except the host name (set once, at the start).
#pragma once

#include <cstdint>

#include "avela_network/model/host_name.hpp"
#include "avela_network/model/network_address.hpp"
#include "avela_network/model/wifi_credentials.hpp"
#include "avela_network/model/wifi_status.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela {

struct NetworkModel {
    static constexpr std::size_t address_capacity = network_address_capacity;

    WifiStatus m_status = WifiStatus::not_configured;
    FixedString<WifiCredentials::ssid_capacity> m_ssid;  // the saved network
    FixedString<address_capacity> m_address;             // on the home network, while connected
    int m_signal_dbm = 0;                                // while connected
    bool m_setup_network_active = false;                 // the radio's own network (for the setup page) is up
    FixedString<address_capacity> m_setup_address;       // the setup page's address on it
    // the name on the home network in force since the start (a newly saved one takes effect at the next); written once,
    // by the service's first update(), before anyone asks for it
    FixedString<HostName::capacity> m_host_name;
    uint32_t m_revision = 0;  // raised whenever a field above changes; readers compare it

    // Signal strength as 0..3 bars, for icons.
    [[nodiscard]] int signal_bars() const
    {
        constexpr int strong_dbm = -60;
        constexpr int fair_dbm = -70;
        if (m_status != WifiStatus::connected) {
            return 0;
        }
        if (m_signal_dbm >= strong_dbm) {
            return 3;
        }
        return m_signal_dbm >= fair_dbm ? 2 : 1;
    }
};

}  // namespace avela
