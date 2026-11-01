// The device's state as the info screen sees it. All fields are reported by the system service; nothing here is
// requested by the screens. Wi-Fi is the network's (avela_network, NetworkModel).
#pragma once

#include <chrono>
#include <cstddef>

namespace avela {

struct SystemInfoModel {
    // reported
    std::size_t m_free_kilobytes = 0;          // internal RAM
    std::size_t m_minimum_free_kilobytes = 0;  // lowest since start
    std::size_t m_total_kilobytes = 0;
    const char* m_firmware_version = "";
    std::chrono::seconds m_uptime{};
};

}  // namespace avela
