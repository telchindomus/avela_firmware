#include "avela_network/service/captive_dns.hpp"

#include <algorithm>
#include <cstddef>
#include <span>

#include "esp_log.h"
#include "lwip/sockets.h"

namespace avela {
namespace {

constexpr const char* log_tag = "dns";
constexpr uint16_t dns_port = 53;
constexpr std::size_t message_capacity = 512;  // classic DNS over UDP
constexpr std::size_t header_size = 12;
constexpr std::size_t answer_size = 16;  // name pointer, type, class, TTL, length, IPv4 address
constexpr int idle_poll_ms = 500;        // while disabled, and the receive timeout
constexpr int microseconds_per_millisecond = 1000;
constexpr uint16_t answer_ttl_seconds = 60;

// header fields (big endian), as byte offsets
constexpr std::size_t flags_offset = 2;
constexpr std::size_t question_count_offset = 4;
constexpr std::size_t answer_count_offset = 6;
constexpr std::size_t authority_count_offset = 8;
constexpr std::size_t additional_count_offset = 10;
// answer record fields, as byte offsets from its start
constexpr std::size_t answer_type_offset = 2;
constexpr std::size_t answer_class_offset = 4;
constexpr std::size_t answer_ttl_offset = 6;  // 32 bits
constexpr std::size_t answer_length_offset = 10;
constexpr std::size_t answer_address_offset = 12;
constexpr std::size_t question_tail_size = 4;  // type and class after the name
constexpr uint16_t response_flags = 0x8180;    // response, recursion desired + available, no error
constexpr uint16_t type_a = 1;
constexpr uint16_t class_internet = 1;
constexpr uint16_t name_pointer_to_question = 0xC00C;
constexpr uint8_t label_pointer_bits = 0xC0;
constexpr unsigned bits_per_byte = 8;
constexpr uint8_t byte_mask = 0xFF;

using Message = std::array<uint8_t, message_capacity>;

//--------------------------------------------------------------------------------------------------

uint16_t read16(const Message& message, std::size_t offset)
{
    return static_cast<uint16_t>((message.at(offset) << bits_per_byte) | message.at(offset + 1));
}

//--------------------------------------------------------------------------------------------------

void write16(Message& message, std::size_t offset, uint16_t value)
{
    message.at(offset) = static_cast<uint8_t>(value >> bits_per_byte);
    message.at(offset + 1) = static_cast<uint8_t>(value & byte_mask);
}

//--------------------------------------------------------------------------------------------------

// Turns a query into its answer in place; returns the answer's length, or 0 to stay silent.
std::size_t answer(Message& message, std::size_t length, uint32_t address)
{
    if (length < header_size || read16(message, question_count_offset) != 1) {
        return 0;
    }
    // the question: name labels up to the empty one, then type and class
    std::size_t offset = header_size;
    while (offset < length && message.at(offset) != 0) {
        if ((message.at(offset) & label_pointer_bits) != 0) {
            return 0;  // compressed names don't occur in questions
        }
        offset += 1 + static_cast<std::size_t>(message.at(offset));
    }
    const std::size_t question_end = offset + 1 + question_tail_size;
    if (question_end > length || question_end + answer_size > message.size()) {
        return 0;
    }
    const bool asks_for_address = read16(message, offset + 1) == type_a;

    write16(message, flags_offset, response_flags);
    write16(message, answer_count_offset, asks_for_address ? 1 : 0);
    write16(message, authority_count_offset, 0);
    write16(message, additional_count_offset, 0);
    if (!asks_for_address) {
        return question_end;  // no error, no answer (AAAA and the rest)
    }
    const std::size_t start = question_end;
    write16(message, start, name_pointer_to_question);
    write16(message, start + answer_type_offset, type_a);
    write16(message, start + answer_class_offset, class_internet);
    write16(message, start + answer_ttl_offset, 0);
    write16(message, start + answer_ttl_offset + 2, answer_ttl_seconds);
    write16(message, start + answer_length_offset, sizeof(address));
    const std::size_t end = start + answer_address_offset;
    std::copy_n(std::as_bytes(std::span{&address, 1}).begin(), sizeof(address),
                std::as_writable_bytes(std::span(message)).subspan(end).begin());  // already in network byte order
    return end + sizeof(address);
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void CaptiveDns::init()
{
    constexpr UBaseType_t priority = 2;
    xTaskCreateStatic(&CaptiveDns::run, task_name, m_stack_.size(), this, priority, m_stack_.data(), &m_task_);
}

//--------------------------------------------------------------------------------------------------

void CaptiveDns::run(void* self)
{
    static_cast<CaptiveDns*>(self)->serve();
}

//--------------------------------------------------------------------------------------------------

void CaptiveDns::serve()
{
    static Message message{};
    int dns_socket = -1;
    while (true) {
        const uint32_t address = m_address_.load();
        if (address == 0) {
            if (dns_socket >= 0) {
                close(dns_socket);
                dns_socket = -1;
            }
            vTaskDelay(pdMS_TO_TICKS(idle_poll_ms));
            continue;
        }
        if (dns_socket < 0) {
            dns_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            sockaddr_in local{};
            local.sin_family = AF_INET;
            local.sin_port = htons(dns_port);
            local.sin_addr.s_addr = htonl(INADDR_ANY);
            const timeval timeout{.tv_sec = 0, .tv_usec = idle_poll_ms * microseconds_per_millisecond};
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): the BSD socket API takes sockaddr*
            if (dns_socket < 0 || bind(dns_socket, reinterpret_cast<const sockaddr*>(&local), sizeof(local)) != 0 ||
                setsockopt(dns_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0) {
                ESP_LOGE(log_tag, "no DNS socket (errno %d)", errno);
                if (dns_socket >= 0) {
                    close(dns_socket);
                    dns_socket = -1;
                }
                vTaskDelay(pdMS_TO_TICKS(idle_poll_ms));
                continue;
            }
        }
        sockaddr_in sender{};
        socklen_t sender_length = sizeof(sender);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): the BSD socket API takes sockaddr*
        auto* const sender_address = reinterpret_cast<sockaddr*>(&sender);
        const auto received = recvfrom(dns_socket, message.data(), message.size(), 0, sender_address, &sender_length);
        if (received <= 0) {
            continue;  // timeout: look at the switch again
        }
        const std::size_t length = answer(message, static_cast<std::size_t>(received), address);
        if (length > 0) {
            sendto(dns_socket, message.data(), length, 0, sender_address, sender_length);
        }
    }
}

}  // namespace avela
