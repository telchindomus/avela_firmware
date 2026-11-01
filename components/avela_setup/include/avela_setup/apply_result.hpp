// What a setup section's apply() did: saved, or why not (for programs: m_error, m_field; for people: m_message).
#pragma once

#include <cstddef>
#include <string_view>

#include "avela_setup/apply_error.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela {

struct ApplyResult {
    static constexpr std::size_t message_capacity = 160;

    bool m_saved = false;
    ApplyError m_error = ApplyError::none;
    std::string_view m_field;                 // the field an error is about (one of the section's names), or empty
    FixedString<message_capacity> m_message;  // shown on the result page and the console

    static ApplyResult invalid(std::string_view field, std::string_view message)
    {
        ApplyResult result;
        result.m_error = ApplyError::invalid_value;
        result.m_field = field;
        result.m_message.append(message);
        return result;
    }
    static ApplyResult storage_failed()
    {
        ApplyResult result;
        result.m_error = ApplyError::storage_failed;
        result.m_message.append("The radio couldn't store it. Please try again.");
        return result;
    }
};

}  // namespace avela
