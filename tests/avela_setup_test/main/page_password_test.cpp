#include "avela_setup/page_password.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <optional>
#include <string>

namespace avela {
namespace {

// "Basic " and the base64 of `credentials`, as a browser sends it.
std::string authorization(std::string_view credentials)
{
    constexpr std::string_view alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded = "Basic ";
    for (std::size_t index = 0; index < credentials.size(); index += 3) {
        const std::size_t count = std::min<std::size_t>(3, credentials.size() - index);
        uint32_t group = 0;
        for (std::size_t offset = 0; offset < 3; ++offset) {
            const auto byte = offset < count ? static_cast<unsigned char>(credentials[index + offset]) : 0U;
            group = (group << 8U) | byte;
        }
        for (std::size_t character = 0; character < 4; ++character) {
            encoded += character <= count ? alphabet[(group >> (18U - 6U * character)) & 0x3FU] : '=';
        }
    }
    return encoded;
}

// The record for `password` with a fixed salt.
PagePassword saved(std::string_view password)
{
    constexpr std::array<uint8_t, PagePassword::salt_bytes> salt{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    const std::optional<PagePassword> record = page_password::hashed(password, salt);
    REQUIRE(record.has_value());
    return record.value_or(PagePassword{});
}

}  // namespace

TEST_CASE("avela-admin with the right password authorizes")
{
    const PagePassword record = saved("secret pass");
    CHECK(page_password::authorizes(authorization("avela-admin:secret pass"), record));
    // every padding length
    CHECK(page_password::authorizes(authorization("avela-admin:12345678"), saved("12345678")));
    CHECK(page_password::authorizes(authorization("avela-admin:123456789"), saved("123456789")));
    CHECK(page_password::authorizes(authorization("avela-admin:1234567890"), saved("1234567890")));
    CHECK(page_password::authorizes(authorization("avela-admin:a:b:c:d:e"), saved("a:b:c:d:e")));
}

TEST_CASE("anything else doesn't")
{
    const PagePassword record = saved("secret pass");
    CHECK_FALSE(page_password::authorizes(authorization("avela-admin:secret pasS"), record));
    CHECK_FALSE(page_password::authorizes(authorization("avela-admin:secret pas"), record));
    CHECK_FALSE(page_password::authorizes(authorization("avela-admin:secret pass!"), record));
    CHECK_FALSE(page_password::authorizes(authorization("avela:secret pass"), record));  // another user name
    CHECK_FALSE(page_password::authorizes(authorization(":secret pass"), record));       // none
    CHECK_FALSE(page_password::authorizes(authorization("Avela-Admin:secret pass"), record));
    CHECK_FALSE(page_password::authorizes(authorization("secret pass"), record));  // no colon
    CHECK_FALSE(page_password::authorizes("", record));
    CHECK_FALSE(page_password::authorizes("Bearer abc", record));
    CHECK_FALSE(page_password::authorizes("Basic not*base64", record));
    CHECK_FALSE(page_password::authorizes("Basic YWJj=", record));  // not a multiple of four
    CHECK_FALSE(page_password::authorizes("Basic YQ===", record));  // three padding characters
    CHECK_FALSE(page_password::authorizes(authorization(std::string(200, 'u') + ":secret pass"), record));  // too long
    CHECK_FALSE(page_password::authorizes(authorization("avela-admin:secret pass"), PagePassword{}));  // a zero record
}

TEST_CASE("the salt changes the hash")
{
    const PagePassword record = saved("secret pass");
    constexpr std::array<uint8_t, PagePassword::salt_bytes> other_salt{};
    const std::optional<PagePassword> other = page_password::hashed("secret pass", other_salt);
    REQUIRE(other.has_value());
    const PagePassword other_record = other.value_or(PagePassword{});
    CHECK(other_record.m_hash != record.m_hash);
    CHECK(page_password::authorizes(authorization("avela-admin:secret pass"), other_record));
}

TEST_CASE("the hash is PBKDF2-HMAC-SHA256 with 1000 rounds")
{
    // Python: hashlib.pbkdf2_hmac("sha256", b"secret pass", bytes(range(1, 17)), 1000, 32)
    constexpr std::array<uint8_t, PagePassword::hash_bytes> expected{
        0x01, 0x82, 0xCF, 0xBF, 0xA8, 0x4B, 0xAD, 0x38, 0x92, 0xC6, 0xE3, 0xAC, 0x94, 0x00, 0x12, 0x1C,
        0x93, 0x07, 0xC2, 0x62, 0x0D, 0x6A, 0x00, 0x04, 0x2E, 0xFF, 0xB0, 0x1B, 0x1C, 0x15, 0x3A, 0x40};
    CHECK(saved("secret pass").m_hash == expected);
}

TEST_CASE("a page password has 8 to 32 printable characters")
{
    CHECK(page_password::valid("12345678"));
    CHECK(page_password::valid("with spaces and ~!@#"));
    CHECK(page_password::valid(std::string(32, 'a')));
    CHECK_FALSE(page_password::valid("1234567"));
    CHECK_FALSE(page_password::valid(std::string(33, 'a')));
    CHECK_FALSE(page_password::valid("tab\tinside"));
    CHECK_FALSE(page_password::valid("non-ASCII \xC3\xA9"));
}

}  // namespace avela
