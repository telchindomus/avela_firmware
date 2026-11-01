// A section's saved values (SetupSection::write_values) on the serial console, for people (text: "  name = value"
// lines and notes) or for programs (json: the "values" object and the "notes" list of a machine reply). Secret fields
// (passwords) are never shown as text; in json only when asked for (a settings dump with secrets), else left out, so a
// restore keeps the saved one.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "avela_strings/fixed_string.hpp"

namespace avela {

class ValueWriter {
public:
    enum class Format : uint8_t { text, json };

    ValueWriter(Format format, bool include_secrets) : m_format_(format), m_include_secrets_(include_secrets) {}

    // Every field once, empty if unset.
    void value(std::string_view name, std::string_view text);
    void secret(std::string_view name, std::string_view text);
    // Something worth knowing that isn't a field (e.g. what the place lookup found).
    void note(std::string_view text);
    // json: closes the values and writes the notes; after the section's write_values().
    void finish();

private:
    static constexpr std::size_t max_notes = 4;
    static constexpr std::size_t note_capacity = 96;

    Format m_format_;
    bool m_include_secrets_;
    bool m_first_value_ = true;
    std::array<FixedString<note_capacity>, max_notes> m_notes_{};
    std::size_t m_note_count_ = 0;
};

// A JSON string literal (quotes and escapes) to the console.
void write_json_string(std::string_view text);

}  // namespace avela
