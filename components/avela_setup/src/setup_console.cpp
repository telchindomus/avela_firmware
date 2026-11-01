#include "avela_setup/setup_console.hpp"

#include <optional>

#include "avela_setup/serial_console.hpp"
#include "esp_system.h"

namespace avela::setup_console {
namespace {

constexpr char space = ' ';
constexpr char quote = '"';
constexpr char backslash = '\\';

using QuotedValue = FixedString<SerialConsole::line_capacity + 1>;

std::string_view trim_start(std::string_view text)
{
    while (!text.empty() && text.front() == space) {
        text.remove_prefix(1);
    }
    return text;
}

void print_line(std::string_view indent, std::string_view text)
{
    SerialConsole::write(indent);
    SerialConsole::write(text);
    SerialConsole::write("\n");
}

// `text` and spaces up to `width` (at least one space after it)
void write_column(std::string_view text, std::size_t width)
{
    constexpr std::string_view spaces = "                    ";
    SerialConsole::write(text);
    SerialConsole::write(spaces.substr(0, text.size() < width ? width - text.size() : 1));
}

constexpr std::size_t name_column = 10;

// A quoted value, from just after its opening quote: \" is a quote and \\ a backslash, any other backslash is itself.
// How many characters it took, the closing quote included; nothing if it has none.
std::optional<std::size_t> read_quoted(std::string_view text, QuotedValue& value)
{
    for (std::size_t index = 0; index < text.size(); ++index) {
        if (text[index] == quote) {
            return index + 1;
        }
        const bool escape = text[index] == backslash && index + 1 < text.size() &&
                            (text[index + 1] == quote || text[index + 1] == backslash);
        if (escape) {
            ++index;
        }
        value.append(text.substr(index, 1));
    }
    return std::nullopt;
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void split_command(std::string_view line, std::string_view& command, std::string_view& arguments)
{
    line = trim_start(line);
    const std::size_t end = line.find(space);
    command = line.substr(0, end);
    arguments = end == std::string_view::npos ? std::string_view{} : trim_start(line.substr(end));
    while (!arguments.empty() && arguments.back() == space) {
        arguments.remove_suffix(1);
    }
}

//--------------------------------------------------------------------------------------------------

bool read_arguments(std::string_view arguments, FormFields& fields, Error& error)
{
    fields.clear();
    arguments = trim_start(arguments);
    while (!arguments.empty()) {
        const std::size_t equals = arguments.find('=');
        const std::size_t space_at = arguments.find(space);
        if (equals == std::string_view::npos || (space_at != std::string_view::npos && space_at < equals)) {
            error.append("Expected name=value, e.g. place=Warsaw (help lists the names).");
            return false;
        }
        const std::string_view name = arguments.substr(0, equals);
        arguments.remove_prefix(equals + 1);
        std::string_view value;
        QuotedValue quoted;
        if (!arguments.empty() && arguments.front() == quote) {
            const std::optional<std::size_t> taken = read_quoted(arguments.substr(1), quoted);
            if (!taken) {
                error.append("A quoted value needs its closing quote.");
                return false;
            }
            value = quoted.view();
            arguments.remove_prefix(1 + *taken);
        } else {
            const std::size_t value_end = arguments.find(space);
            value = arguments.substr(0, value_end);
            arguments.remove_prefix(value.size());
        }
        if (!fields.add(name, value)) {
            error.append("Too many or too long values.");
            return false;
        }
        arguments = trim_start(arguments);
    }
    return true;
}

//--------------------------------------------------------------------------------------------------

std::string_view unknown_field(const FormFields& fields, std::span<const SetupField> known)
{
    for (std::size_t index = 0; index < fields.size(); ++index) {
        const std::string_view name = fields.name(index);
        bool found = false;
        for (const SetupField& field : known) {
            found = found || field.m_name == name;
        }
        if (!found) {
            return name;
        }
    }
    return {};
}

//--------------------------------------------------------------------------------------------------

void print(std::string_view text)
{
    print_line("", text);
}

//--------------------------------------------------------------------------------------------------

void print_help_start()
{
    print_line("", "Commands (and :sections, :get, :set, :restart for programs, see the avela CLI):");
    print_line("  ", "help                      this list");
    print_line("  ", "<section>                 its saved values");
    print_line("  ", "<section> name=value ...  change them; a value with spaces in quotes: place=\"New York\"");
    print_line("  ", R"(                          (in quotes, \" is a quote and \\ a backslash))");
    print_line("  ", "restart                   restart the radio");
    print_line("", "Sections:");
}

//--------------------------------------------------------------------------------------------------

void print_help_section(std::string_view id, std::string_view title, std::span<const SetupField> fields)
{
    SerialConsole::write("  ");
    write_column(id, name_column);
    print_line("", title);
    for (const SetupField& field : fields) {
        SerialConsole::write("    ");
        write_column(field.m_name, name_column);
        print_line("", field.m_description);
    }
}

//--------------------------------------------------------------------------------------------------

void print_result(const ApplyResult& result)
{
    print_line(result.m_saved ? "" : "Not saved: ", result.m_message.view());
}

//--------------------------------------------------------------------------------------------------

void reply_ok()
{
    const SerialConsole::OutputLock lock;
    SerialConsole::write("{\"ok\":true}\n");
}

//--------------------------------------------------------------------------------------------------

void reply_error(std::string_view error, std::string_view field, std::string_view message)
{
    const SerialConsole::OutputLock lock;
    SerialConsole::write(R"({"ok":false,"error":)");
    write_json_string(error);
    SerialConsole::write(",\"field\":");
    write_json_string(field);
    SerialConsole::write(",\"message\":");
    write_json_string(message);
    SerialConsole::write("}\n");
}

//--------------------------------------------------------------------------------------------------

void reply_result(const ApplyResult& result)
{
    const SerialConsole::OutputLock lock;
    switch (result.m_error) {
    case ApplyError::none:
        SerialConsole::write(R"({"ok":true,"message":)");
        write_json_string(result.m_message.view());
        SerialConsole::write("}\n");
        return;
    case ApplyError::invalid_value:
        reply_error("invalid_value", result.m_field, result.m_message.view());
        return;
    case ApplyError::storage_failed:
        reply_error("storage_failed", result.m_field, result.m_message.view());
        return;
    case ApplyError::unknown_field:
        reply_error("unknown_field", result.m_field, result.m_message.view());
        return;
    }
}

//--------------------------------------------------------------------------------------------------

void reply_sections_start()
{
    FixedString<capacity_for(R"({"ok":true,"protocol":)") + number_capacity<int>> start;
    start.append(R"({"ok":true,"protocol":)").append_number(protocol_version);
    SerialConsole::write(start.view());
    SerialConsole::write(",\"sections\":[");
}

//--------------------------------------------------------------------------------------------------

void reply_section(bool first, std::string_view id, std::string_view title, std::span<const SetupField> fields)
{
    SerialConsole::write(first ? "{\"id\":" : ",{\"id\":");
    write_json_string(id);
    SerialConsole::write(",\"title\":");
    write_json_string(title);
    SerialConsole::write(",\"fields\":[");
    bool first_field = true;
    for (const SetupField& field : fields) {
        SerialConsole::write(first_field ? "{\"name\":" : ",{\"name\":");
        first_field = false;
        write_json_string(field.m_name);
        SerialConsole::write(",\"description\":");
        write_json_string(field.m_description);
        SerialConsole::write(field.m_secret ? ",\"secret\":true}" : ",\"secret\":false}");
    }
    SerialConsole::write("]}");
}

//--------------------------------------------------------------------------------------------------

void reply_sections_end()
{
    SerialConsole::write("]}\n");
}

//--------------------------------------------------------------------------------------------------

void reply_values_start()
{
    SerialConsole::write(R"({"ok":true,"values":{)");
}

//--------------------------------------------------------------------------------------------------

void reply_values_end()
{
    SerialConsole::write("}\n");
}

//--------------------------------------------------------------------------------------------------

void restart()
{
    print_line("", "Restarting.");
    esp_restart();
}

}  // namespace avela::setup_console
