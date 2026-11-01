#include "avela_setup/page_password.hpp"

#include <algorithm>
#include <array>
#include <optional>

#include "avela_strings/fixed_string.hpp"
#include "mbedtls/md.h"
#include "mbedtls/pkcs5.h"

namespace avela::page_password {
namespace {

constexpr std::string_view basic_prefix = "Basic ";
// "avela-admin:<password>" decoded; anything longer can't be right
constexpr std::size_t credentials_capacity = PagePassword::user_name.size() + 1 + PagePassword::maximum_length + 1;

// The value (0..63) of a base64 character, nothing for any other character.
std::optional<uint32_t> base64_value(char character)
{
    constexpr uint32_t upper_start = 0;
    constexpr uint32_t lower_start = 26;
    constexpr uint32_t digit_start = 52;
    constexpr uint32_t plus_value = 62;
    constexpr uint32_t slash_value = 63;
    if (character >= 'A' && character <= 'Z') {
        return upper_start + static_cast<uint32_t>(character - 'A');
    }
    if (character >= 'a' && character <= 'z') {
        return lower_start + static_cast<uint32_t>(character - 'a');
    }
    if (character >= '0' && character <= '9') {
        return digit_start + static_cast<uint32_t>(character - '0');
    }
    if (character == '+') {
        return plus_value;
    }
    if (character == '/') {
        return slash_value;
    }
    return std::nullopt;
}

// Decodes base64 (with its '=' padding at the end) into `decoded`; false if it isn't base64 or doesn't fit.
bool decode_base64(std::string_view encoded, FixedString<credentials_capacity>& decoded)
{
    constexpr std::size_t bits_per_character = 6;
    constexpr std::size_t bits_per_byte = 8;
    constexpr uint32_t byte_mask = 0xFF;
    constexpr std::size_t group_characters = 4;
    if (encoded.size() % group_characters != 0) {
        return false;
    }
    const std::size_t padding = encoded.size() - std::min(encoded.size(), encoded.find_last_not_of('=') + 1);
    if (padding > 2) {
        return false;
    }
    encoded.remove_suffix(padding);
    uint32_t bits = 0;
    std::size_t bit_count = 0;
    for (const char character : encoded) {
        const std::optional<uint32_t> value = base64_value(character);
        if (!value) {
            return false;
        }
        bits = (bits << bits_per_character) | *value;
        bit_count += bits_per_character;
        if (bit_count >= bits_per_byte) {
            bit_count -= bits_per_byte;
            if (decoded.view().size() == credentials_capacity - 1) {
                return false;
            }
            const std::array byte{static_cast<char>((bits >> bit_count) & byte_mask)};
            decoded.append({byte.data(), byte.size()});
        }
    }
    return true;
}

}  // namespace

//--------------------------------------------------------------------------------------------------

bool valid(std::string_view password)
{
    constexpr char first_printable = ' ';
    constexpr char last_printable = '~';
    return password.size() >= PagePassword::minimum_length && password.size() <= PagePassword::maximum_length &&
           std::ranges::all_of(
               password, [](char character) { return character >= first_printable && character <= last_printable; });
}

//--------------------------------------------------------------------------------------------------

std::optional<PagePassword> hashed(std::string_view password, std::span<const uint8_t, PagePassword::salt_bytes> salt)
{
    PagePassword record{};
    std::ranges::copy(salt, record.m_salt.begin());
    std::array<uint8_t, PagePassword::maximum_length> bytes{};
    const std::size_t length = std::min(password.size(), bytes.size());
    std::transform(password.begin(), std::next(password.begin(), static_cast<std::ptrdiff_t>(length)), bytes.begin(),
                   [](char character) { return static_cast<uint8_t>(character); });
    if (mbedtls_pkcs5_pbkdf2_hmac_ext(MBEDTLS_MD_SHA256, bytes.data(), length, record.m_salt.data(),
                                      record.m_salt.size(), PagePassword::iterations,
                                      static_cast<uint32_t>(record.m_hash.size()), record.m_hash.data()) != 0) {
        return std::nullopt;
    }
    return record;
}

//--------------------------------------------------------------------------------------------------

bool authorizes(std::string_view authorization, const PagePassword& saved)
{
    if (!authorization.starts_with(basic_prefix)) {
        return false;
    }
    FixedString<credentials_capacity> credentials;
    if (!decode_base64(authorization.substr(basic_prefix.size()), credentials)) {
        return false;
    }
    const std::size_t colon = credentials.view().find(':');
    if (colon == std::string_view::npos) {
        return false;
    }
    const bool right_user = credentials.view().substr(0, colon) == PagePassword::user_name;  // public: no secret
    const std::string_view given = credentials.view().substr(colon + 1);
    if (!valid(given)) {
        return false;
    }
    const std::optional<PagePassword> candidate = hashed(given, saved.m_salt);
    if (!candidate) {
        return false;
    }
    // every byte compared, wherever the first difference is
    uint32_t difference = 0;
    for (std::size_t index = 0; index < saved.m_hash.size(); ++index) {
        difference |= static_cast<uint32_t>(candidate->m_hash.at(index) ^ saved.m_hash.at(index));
    }
    return right_user && difference == 0;
}

}  // namespace avela::page_password
