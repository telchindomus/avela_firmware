// Whether a connection to the setup server comes from the radio's own setup network, which needs no page password
// (page_password.hpp). All three must hold, else the password is asked for:
// - the client's address is in the setup network;
// - it connected to the radio's address on the setup network: lwIP takes a packet for any of the radio's addresses on
//   any interface, but the radio answers its setup address only on the setup network, so a client elsewhere gets no
//   reply;
// - the home network doesn't overlap the setup network: if it does (a home network on 192.168.4.x), a home client
//   could have a setup-network address and be answered at the home address, so the check fails closed and the page
//   asks for the password on both networks (USB stays the way back in).
// Addresses are IPv4 in network byte order, as lwIP and esp_netif hold them. Platform-free.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace avela {

struct Ipv4Network {
    uint32_t m_address = 0;  // the radio's own address in it
    uint32_t m_netmask = 0;
};

namespace setup_network_client {

inline constexpr std::size_t ipv6_bytes = 16;

[[nodiscard]] bool is_setup_network_client(uint32_t client, uint32_t connected_to, Ipv4Network setup_network,
                                           std::optional<Ipv4Network> home_network);

// The IPv4 address in an IPv6 one (IPv4-mapped, ::ffff:a.b.c.d: an IPv4 client of an IPv6 socket); nothing for a
// native IPv6 address.
[[nodiscard]] std::optional<uint32_t> mapped_ipv4(std::span<const uint8_t, ipv6_bytes> ipv6);

}  // namespace setup_network_client

}  // namespace avela
