// FixedString<N>: fixed-capacity text for labels built at run time ("Thu, 16 Oct", "07 / 12"): no heap, no printf
// varargs. Always zero-terminated; text that does not fit is cut off.
#pragma once

#include <algorithm>
#include <array>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <limits>
#include <string_view>

namespace avela {

// Capacity (terminator included) for text never longer than `longest`: FixedString<capacity_for("00:00")>.
consteval std::size_t capacity_for(std::string_view longest)
{
    return longest.size() + 1;
}

// Capacity for any value of an integer type: its longest digit string, a sign and the terminator.
template <std::integral Number>
inline constexpr std::size_t number_capacity = std::numeric_limits<Number>::digits10 + 1 + 1 + 1;

template <std::size_t Capacity>
class FixedString {
public:
    FixedString& append(std::string_view text)
    {
        const std::size_t count = std::min(text.size(), Capacity - 1 - m_length_);
        std::copy_n(text.begin(), count, std::next(m_characters_.begin(), static_cast<std::ptrdiff_t>(m_length_)));
        m_length_ += count;
        m_characters_.at(m_length_) = '\0';
        return *this;
    }

    // Decimal number, padded with leading zeros to `minimum_digits` (2 -> "07").
    template <std::integral Number>
    FixedString& append_number(Number value, std::size_t minimum_digits = 1)
    {
        std::array<char, number_capacity<Number>> digits{};
        const auto result = std::to_chars(digits.begin(), digits.end(), value);
        const std::string_view number(digits.data(),
                                      static_cast<std::size_t>(std::distance(digits.begin(), result.ptr)));
        const bool negative = number.starts_with('-');
        if (negative) {
            append("-");
        }
        const std::string_view magnitude = negative ? number.substr(1) : number;
        for (std::size_t padding = magnitude.size(); padding < minimum_digits; ++padding) {
            append("0");
        }
        return append(magnitude);
    }

    FixedString& clear()
    {
        m_length_ = 0;
        m_characters_.front() = '\0';
        return *this;
    }

    [[nodiscard]] const char* c_str() const { return m_characters_.data(); }
    [[nodiscard]] std::string_view view() const { return {m_characters_.data(), m_length_}; }
    [[nodiscard]] bool empty() const { return m_length_ == 0; }
    [[nodiscard]] static constexpr std::size_t max_length() { return Capacity - 1; }

    friend bool operator==(const FixedString& left, const FixedString& right) { return left.view() == right.view(); }

private:
    static_assert(Capacity > 0);
    std::array<char, Capacity> m_characters_{};
    std::size_t m_length_ = 0;
};

}  // namespace avela
