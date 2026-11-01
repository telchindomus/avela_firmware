// The home network to join, as saved by the setup page's Wi-Fi section (a Settings record, avela_settings).
// Sizes are Wi-Fi's own limits: network name up to 32 bytes, WPA2 password 8 to 63 characters (empty = open).
#pragma once

#include <cstdint>

#include "avela_strings/fixed_string.hpp"

namespace avela {

struct WifiCredentials {
    static constexpr const char* key = "wifi";
    static constexpr uint16_t version = 1;
    static constexpr std::size_t ssid_capacity = 32 + 1;
    static constexpr std::size_t password_capacity = 63 + 1;
    static constexpr std::size_t minimum_password_length = 8;

    FixedString<ssid_capacity> m_ssid;
    FixedString<password_capacity> m_password;
};

}  // namespace avela
