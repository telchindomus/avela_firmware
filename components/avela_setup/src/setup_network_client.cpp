#include "avela_setup/setup_network_client.hpp"

#include <algorithm>
#include <cstring>

namespace avela::setup_network_client {

//--------------------------------------------------------------------------------------------------

bool is_setup_network_client(uint32_t client, uint32_t connected_to, Ipv4Network setup_network,
                             std::optional<Ipv4Network> home_network)
{
    if (setup_network.m_address == 0 || connected_to != setup_network.m_address ||
        (client & setup_network.m_netmask) != (setup_network.m_address & setup_network.m_netmask)) {
        return false;
    }
    if (!home_network || home_network->m_address == 0) {
        return true;
    }
    // two networks overlap when they agree on the bits both masks cover
    const uint32_t common_mask = setup_network.m_netmask & home_network->m_netmask;
    return (setup_network.m_address & common_mask) != (home_network->m_address & common_mask);
}

//--------------------------------------------------------------------------------------------------

std::optional<uint32_t> mapped_ipv4(std::span<const uint8_t, ipv6_bytes> ipv6)
{
    constexpr std::array<uint8_t, 12> mapped_prefix{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF};
    if (!std::equal(mapped_prefix.begin(), mapped_prefix.end(), ipv6.begin())) {
        return std::nullopt;
    }
    uint32_t address = 0;
    std::memcpy(&address, ipv6.subspan<mapped_prefix.size()>().data(), sizeof(address));
    return address;
}

}  // namespace avela::setup_network_client
