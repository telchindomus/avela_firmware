#include "avela_setup/form_fields.hpp"

#include <algorithm>
#include <iterator>
#include <span>

namespace avela {
namespace {

constexpr int hex_digit_none = -1;
constexpr int decimal_digits = 10;
constexpr int hex_base = 16;

//--------------------------------------------------------------------------------------------------

int hex_value(char character)
{
    if (character >= '0' && character <= '9') {
        return character - '0';
    }
    if (character >= 'a' && character <= 'f') {
        return character - 'a' + decimal_digits;
    }
    if (character >= 'A' && character <= 'F') {
        return character - 'A' + decimal_digits;
    }
    return hex_digit_none;
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void FormFields::parse(std::size_t length)
{
    // Decoding never makes text longer, so the decoded text is written over the encoded text, behind the read
    // position, and the fields point into the buffer.
    const std::size_t end = std::min(length, m_buffer_.size());
    const char* const base = m_buffer_.data();
    auto view = [&](std::size_t from, std::size_t to) {
        return std::string_view(std::next(base, static_cast<std::ptrdiff_t>(from)), to - from);
    };
    std::size_t read = 0;
    std::size_t write = 0;
    m_count_ = 0;
    while (read < end) {
        const std::size_t name_start = write;
        std::optional<std::size_t> value_start;  // set at the first '='
        for (; read < end && m_buffer_[read] != '&'; ++read) {
            char character = m_buffer_[read];
            if (character == '=' && !value_start) {
                value_start = write;
                continue;
            }
            if (character == '+') {
                character = ' ';
            } else if (character == '%' && read + 2 < end) {
                const int high = hex_value(m_buffer_[read + 1]);
                const int low = hex_value(m_buffer_[read + 2]);
                if (high != hex_digit_none && low != hex_digit_none) {
                    character = static_cast<char>(high * hex_base + low);
                    read += 2;
                }
            }
            m_buffer_[write] = character;
            ++write;
        }
        ++read;  // past the '&'
        const std::size_t name_end = value_start.value_or(write);
        if (m_count_ < m_fields_.size() && name_end > name_start) {
            m_fields_.at(m_count_) = Field{.m_name = view(name_start, name_end), .m_value = view(name_end, write)};
            ++m_count_;
        }
    }
}

//--------------------------------------------------------------------------------------------------

void FormFields::clear()
{
    m_count_ = 0;
    m_used_ = 0;
}

//--------------------------------------------------------------------------------------------------

bool FormFields::add(std::string_view name, std::string_view value)
{
    if (m_count_ >= m_fields_.size() || name.size() + value.size() > m_buffer_.size() - m_used_) {
        return false;
    }
    const auto copy = [this](std::string_view text) {
        const std::span<char> target = m_buffer_.subspan(m_used_, text.size());
        std::ranges::copy(text, target.begin());
        m_used_ += text.size();
        return std::string_view{target.data(), target.size()};
    };
    const std::string_view stored_name = copy(name);
    const std::string_view stored_value = copy(value);
    m_fields_.at(m_count_) = Field{.m_name = stored_name, .m_value = stored_value};
    ++m_count_;
    return true;
}

//--------------------------------------------------------------------------------------------------

std::optional<std::string_view> FormFields::find(std::string_view name) const
{
    const auto fields = std::span(m_fields_).first(m_count_);
    const auto field = std::ranges::find_if(fields, [&](const Field& candidate) { return candidate.m_name == name; });
    if (field == fields.end()) {
        return std::nullopt;
    }
    return field->m_value;
}

}  // namespace avela
