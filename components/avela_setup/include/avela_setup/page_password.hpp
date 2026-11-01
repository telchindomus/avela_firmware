// The setup page's password on the home network (a Settings record, avela_settings): the browser asks for it (HTTP
// Basic authentication, user name avela-admin; a server can't fill in the browser's prompt, so the prompt's title
// and the docs name it). It is set on the radio's own setup network or over USB (the
// Access section), both only for someone at the radio; until then the page doesn't open on the home network.
// Only a salted PBKDF2-SHA256 hash is stored, never the password, so it can't be read from the flash; a copy of the
// flash still lets someone try guesses against the hash on a fast computer, so the page password shouldn't be one used
// elsewhere.
// The page is plain HTTP: the password keeps out others on the home network, not someone recording its traffic.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace avela {

struct PagePassword {
    static constexpr const char* key = "page_password";
    static constexpr std::string_view user_name = "avela-admin";
    static constexpr uint16_t version = 1;
    static constexpr std::size_t minimum_length = 8;
    static constexpr std::size_t maximum_length = 32;
    static constexpr std::size_t salt_bytes = 16;
    static constexpr std::size_t hash_bytes = 32;
    // PBKDF2 rounds: every request is checked, so a page waits for them; a guess from a copy of the flash costs as much
    static constexpr uint32_t iterations = 1000;

    std::array<uint8_t, salt_bytes> m_salt{};
    std::array<uint8_t, hash_bytes> m_hash{};
};

namespace page_password {

// 8 to 32 printable ASCII characters (spaces too).
[[nodiscard]] bool valid(std::string_view password);

// The record for `password` with this salt (random, from the caller); nothing if the hashing failed.
[[nodiscard]] std::optional<PagePassword> hashed(std::string_view password,
                                                 std::span<const uint8_t, PagePassword::salt_bytes> salt);

// Whether an Authorization header ("Basic " and base64 of "avela-admin:<password>") carries the user name and the saved
// password. Compares the hashes in a time that doesn't depend on where they differ.
[[nodiscard]] bool authorizes(std::string_view authorization, const PagePassword& saved);

}  // namespace page_password

}  // namespace avela
