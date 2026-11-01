#include "avela_setup/setup_network_client.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <cstring>
#include <optional>

namespace avela {
namespace {

// a.b.c.d in network byte order, as lwIP holds it (the first byte first in memory)
uint32_t ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    const std::array<uint8_t, 4> bytes{a, b, c, d};
    uint32_t address = 0;
    std::memcpy(&address, bytes.data(), sizeof(address));
    return address;
}

const uint32_t mask24 = ip(255, 255, 255, 0);
const Ipv4Network setup_network{.m_address = ip(192, 168, 4, 1), .m_netmask = mask24};
const Ipv4Network home_network{.m_address = ip(192, 168, 1, 50), .m_netmask = mask24};

}  // namespace

TEST_CASE("a client on the setup network, at the setup address")
{
    CHECK(setup_network_client::is_setup_network_client(ip(192, 168, 4, 2), ip(192, 168, 4, 1), setup_network,
                                                        home_network));
    CHECK(setup_network_client::is_setup_network_client(ip(192, 168, 4, 2), ip(192, 168, 4, 1), setup_network,
                                                        std::nullopt));  // no home network yet
}

TEST_CASE("a home client while the setup network is open")
{
    CHECK_FALSE(setup_network_client::is_setup_network_client(ip(192, 168, 1, 7), ip(192, 168, 1, 50), setup_network,
                                                              home_network));
    // asking for the setup address from the home network
    CHECK_FALSE(setup_network_client::is_setup_network_client(ip(192, 168, 1, 7), ip(192, 168, 4, 1), setup_network,
                                                              home_network));
}

TEST_CASE("a setup-network address that came to the home address")
{
    CHECK_FALSE(setup_network_client::is_setup_network_client(ip(192, 168, 4, 2), ip(192, 168, 1, 50), setup_network,
                                                              home_network));
}

TEST_CASE("a home network overlapping the setup network fails closed")
{
    const Ipv4Network same_subnet{.m_address = ip(192, 168, 4, 100), .m_netmask = mask24};
    CHECK_FALSE(setup_network_client::is_setup_network_client(ip(192, 168, 4, 10), ip(192, 168, 4, 100), setup_network,
                                                              same_subnet));
    CHECK_FALSE(setup_network_client::is_setup_network_client(ip(192, 168, 4, 10), ip(192, 168, 4, 1), setup_network,
                                                              same_subnet));
    const Ipv4Network same_address{.m_address = ip(192, 168, 4, 1), .m_netmask = mask24};
    CHECK_FALSE(setup_network_client::is_setup_network_client(ip(192, 168, 4, 10), ip(192, 168, 4, 1), setup_network,
                                                              same_address));
    // a wider home network around the setup network
    const Ipv4Network wider{.m_address = ip(192, 168, 0, 9), .m_netmask = ip(255, 255, 0, 0)};
    CHECK_FALSE(
        setup_network_client::is_setup_network_client(ip(192, 168, 4, 10), ip(192, 168, 4, 1), setup_network, wider));
}

TEST_CASE("no setup network")
{
    CHECK_FALSE(setup_network_client::is_setup_network_client(ip(192, 168, 4, 2), ip(192, 168, 4, 1), Ipv4Network{},
                                                              home_network));
}

TEST_CASE("IPv4 clients of the IPv6 socket")
{
    const std::array<uint8_t, 16> mapped{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 192, 168, 4, 2};
    CHECK(setup_network_client::mapped_ipv4(mapped) == ip(192, 168, 4, 2));
    const std::array<uint8_t, 16> native{0xFE, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 192, 168, 4, 2};
    CHECK_FALSE(setup_network_client::mapped_ipv4(native).has_value());
}

}  // namespace avela
