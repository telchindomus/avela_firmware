// The radio's name on the home network (DHCP host name and mDNS: http://<name>.local), as saved by the setup page's
// Wi-Fi section (a Settings record, avela_settings); without one, menuconfig's (Avela network). A saved name takes
// effect at the next start: the network service sets it once, before the network is up.
#pragma once

#include <cstdint>
#include <string_view>

#include "avela_strings/fixed_string.hpp"

namespace avela {

struct HostName {
    static constexpr const char* key = "host_name";
    static constexpr uint16_t version = 1;
    static constexpr std::size_t capacity = 31 + 1;  // a DNS label may have 63; ESP-IDF's DHCP client sends up to 32

    FixedString<capacity> m_name;
};

// The saved name, else menuconfig's. Any task.
[[nodiscard]] HostName saved_host_name();

// Whether `name` can be a host name: 1 to 31 lowercase letters, digits and '-', no '-' at either end. Platform-free.
[[nodiscard]] bool valid_host_name(std::string_view name);

}  // namespace avela
