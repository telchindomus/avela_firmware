#include "avela_setup/value_writer.hpp"

#include <span>

#include "avela_setup/serial_console.hpp"

namespace avela {

//--------------------------------------------------------------------------------------------------

void ValueWriter::value(std::string_view name, std::string_view text)
{
    if (m_format_ == Format::text) {
        SerialConsole::write("  ");
        SerialConsole::write(name);
        SerialConsole::write(" = ");
        SerialConsole::write(text);
        SerialConsole::write("\n");
        return;
    }
    if (!m_first_value_) {
        SerialConsole::write(",");
    }
    m_first_value_ = false;
    write_json_string(name);
    SerialConsole::write(":");
    write_json_string(text);
}

//--------------------------------------------------------------------------------------------------

void ValueWriter::secret(std::string_view name, std::string_view text)
{
    if (m_format_ == Format::text) {
        value(name, text.empty() ? "(none)" : "(set, not shown)");
    } else if (m_include_secrets_) {
        value(name, text);
    }
}

//--------------------------------------------------------------------------------------------------

void ValueWriter::note(std::string_view text)
{
    if (m_format_ == Format::text) {
        SerialConsole::write("  ");
        SerialConsole::write(text);
        SerialConsole::write("\n");
        return;
    }
    if (m_note_count_ < m_notes_.size()) {
        m_notes_.at(m_note_count_).clear().append(text);
        ++m_note_count_;
    }
}

//--------------------------------------------------------------------------------------------------

void ValueWriter::finish()
{
    if (m_format_ == Format::text) {
        return;
    }
    SerialConsole::write("},\"notes\":[");
    for (std::size_t index = 0; index < m_note_count_; ++index) {
        if (index > 0) {
            SerialConsole::write(",");
        }
        write_json_string(m_notes_.at(index).view());
    }
    SerialConsole::write("]");
}

//--------------------------------------------------------------------------------------------------

void write_json_string(std::string_view text)
{
    constexpr std::string_view hex_digits = "0123456789abcdef";
    constexpr unsigned char first_printable = 0x20;
    constexpr unsigned high_nibble_shift = 4;
    constexpr unsigned nibble_mask = 0x0F;
    SerialConsole::write("\"");
    std::size_t plain_start = 0;
    for (std::size_t index = 0; index < text.size(); ++index) {
        const char character = text[index];
        const auto byte = static_cast<unsigned char>(character);
        if (character != '"' && character != '\\' && byte >= first_printable) {
            continue;  // written with its neighbours
        }
        SerialConsole::write(text.substr(plain_start, index - plain_start));
        if (character == '"' || character == '\\') {
            const std::array<char, 2> escaped{'\\', character};
            SerialConsole::write(std::string_view{escaped.data(), escaped.size()});
        } else {
            const std::array<char, 6> escaped{'\\',
                                              'u',
                                              '0',
                                              '0',
                                              hex_digits[(byte >> high_nibble_shift) & nibble_mask],
                                              hex_digits[byte & nibble_mask]};
            SerialConsole::write(std::string_view{escaped.data(), escaped.size()});
        }
        plain_start = index + 1;
    }
    SerialConsole::write(text.substr(plain_start));
    SerialConsole::write("\"");
}

}  // namespace avela
