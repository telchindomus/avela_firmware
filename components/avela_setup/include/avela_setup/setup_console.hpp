// The settings on the USB serial port: the same sections as the web setup page (SetupServer), read and written as
// text by people, or as JSON by programs (the avela CLI). Commands for people:
//   help                          the commands and every section's fields
//   <section>                     its saved values (e.g. "weather")
//   <section> name=value ...      change them, through the section's apply(): the page's rules and messages
//                                 (a value with spaces in quotes: place="New York"; in quotes, \" is a quote and
//                                 \\ a backslash: ssid="My \"Home\"")
//   restart                       restart the radio
// Machine commands, each answered by one JSON line ({"ok":true,...} or {"ok":false,"error":...}):
//   :sections                     {"ok":true,"protocol":2,"sections":[{"id","title","fields":[{"name","description",
//                                 "secret"}]}]}
//   :get <section> [secrets]      {"ok":true,"values":{name:value},"notes":[...]} (secret fields only with "secrets")
//   :set <section> name=value ... {"ok":true,"message":...} or {"ok":false,"error":"invalid_value"|"unknown_field"|
//                                 "storage_failed"|"invalid_arguments","field":...,"message":...}
//   :restart                      {"ok":true}, then the radio restarts
// Errors for any command: "unknown_command", "unknown_section". Connect with any serial terminal at the console's
// speed (115200), e.g. `idf.py monitor`. Runs on its own task.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "avela_setup/apply_lock.hpp"
#include "avela_setup/apply_result.hpp"
#include "avela_setup/form_fields.hpp"
#include "avela_setup/serial_console.hpp"
#include "avela_setup/setup_field.hpp"
#include "avela_setup/setup_section_concept.hpp"
#include "avela_setup/value_writer.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela {

namespace setup_console {

// Plain functions shared by every SetupConsole instantiation (src/setup_console.cpp).
inline constexpr int protocol_version = 2;  // of the machine commands; raise it when a command or a reply changes
                                            // (2: escapes in quoted values)
using Error = FixedString<ApplyResult::message_capacity>;

// The line's first word and the rest after it.
void split_command(std::string_view line, std::string_view& command, std::string_view& arguments);
// name=value pairs (a value in quotes may have spaces) into `fields`; false (and why in `error`) if they don't fit.
bool read_arguments(std::string_view arguments, FormFields& fields, Error& error);
// The first given name that isn't one of the section's fields, or empty.
std::string_view unknown_field(const FormFields& fields, std::span<const SetupField> known);

// for people
void print(std::string_view text);
void print_help_start();
void print_help_section(std::string_view id, std::string_view title, std::span<const SetupField> fields);
void print_result(const ApplyResult& result);

// for programs: one JSON line each
void reply_ok();
void reply_error(std::string_view error, std::string_view field, std::string_view message);
void reply_result(const ApplyResult& result);
void reply_sections_start();
void reply_section(bool first, std::string_view id, std::string_view title, std::span<const SetupField> fields);
void reply_sections_end();
void reply_values_start();
void reply_values_end();

[[noreturn]] void restart();

}  // namespace setup_console

// The console lists the sections in the order given here; give it the same list as the SetupServer.
template <SetupSectionConcept... Sections>
class SetupConsole {
public:
    explicit SetupConsole(Sections... sections) : m_sections_{sections...} {}
    SetupConsole(const SetupConsole&) = delete;
    SetupConsole& operator=(const SetupConsole&) = delete;
    SetupConsole(SetupConsole&&) = delete;
    SetupConsole& operator=(SetupConsole&&) = delete;
    ~SetupConsole() = default;

    // After Settings::init(). Once.
    void init() { m_serial_.init(); }

private:
    enum class Command : uint8_t { show, change };

    static void handle_line(void* context, std::string_view line) { static_cast<SetupConsole*>(context)->handle(line); }

    void handle(std::string_view line)
    {
        std::string_view command;
        std::string_view arguments;
        setup_console::split_command(line, command, arguments);
        if (command.empty()) {
            return;
        }
        if (command.front() == ':') {
            handle_machine(command.substr(1), arguments);
            return;
        }
        if (command == "help") {
            setup_console::print_help_start();
            for_each_section([](const auto& section, bool /*first*/) {
                using Section = std::remove_cvref_t<decltype(section)>;
                setup_console::print_help_section(Section::id, Section::title, Section::fields);
            });
            return;
        }
        if (command == "restart") {
            setup_console::restart();
        }
        if (!run_section(command, arguments, arguments.empty() ? Command::show : Command::change, false)) {
            setup_console::print("Unknown command; type help.");
        }
    }

    void handle_machine(std::string_view command, std::string_view text)
    {
        if (command == "sections") {
            const SerialConsole::OutputLock lock;  // one JSON line: no log line inside it
            setup_console::reply_sections_start();
            for_each_section([](const auto& section, bool first) {
                using Section = std::remove_cvref_t<decltype(section)>;
                setup_console::reply_section(first, Section::id, Section::title, Section::fields);
            });
            setup_console::reply_sections_end();
            return;
        }
        if (command == "restart") {
            setup_console::reply_ok();
            setup_console::restart();
        }
        if (command != "get" && command != "set") {
            setup_console::reply_error("unknown_command", "", "Unknown command.");
            return;
        }
        std::string_view section_id;
        std::string_view section_arguments;
        setup_console::split_command(text, section_id, section_arguments);
        if (!run_section(section_id, section_arguments, command == "get" ? Command::show : Command::change, true)) {
            setup_console::reply_error("unknown_section", "", "Unknown section.");
        }
    }

    // Shows or changes the section with this id; false if there is none.
    bool run_section(std::string_view id, std::string_view arguments, Command command, bool machine)
    {
        bool found = false;
        std::apply(
            [&](auto&... section) {
                ((found = found || (id == std::remove_reference_t<decltype(section)>::id &&
                                    (run(section, arguments, command, machine), true))),
                 ...);
            },
            m_sections_);
        return found;
    }

    template <typename Section>
    void run(Section& section, std::string_view arguments, Command command, bool machine)
    {
        if (command == Command::show) {
            const bool secrets = machine && arguments == "secrets";
            const SerialConsole::OutputLock lock;  // one reply: no log line inside it
            ValueWriter values{machine ? ValueWriter::Format::json : ValueWriter::Format::text, secrets};
            if (machine) {
                setup_console::reply_values_start();
            }
            section.write_values(values);
            values.finish();
            if (machine) {
                setup_console::reply_values_end();
            }
            return;
        }
        setup_console::Error error;
        if (!setup_console::read_arguments(arguments, m_fields_, error)) {
            report(ApplyResult::invalid("", error.view()), machine, "invalid_arguments");
            return;
        }
        if (const std::string_view unknown = setup_console::unknown_field(m_fields_, Section::fields);
            !unknown.empty()) {
            ApplyResult result;
            result.m_error = ApplyError::unknown_field;
            result.m_field = unknown;
            result.m_message.append("No field ").append(unknown).append(" here; help lists the fields.");
            report(result, machine, "");
            return;
        }
        ApplyResult result;
        {
            const ApplyLock lock;
            result = section.apply(m_fields_);
        }
        report(result, machine, "");
    }

    // `error` overrides the result's error name (for the console's own errors)
    static void report(const ApplyResult& result, bool machine, std::string_view error)
    {
        if (!machine) {
            setup_console::print_result(result);
        } else if (!error.empty()) {
            setup_console::reply_error(error, result.m_field, result.m_message.view());
        } else {
            setup_console::reply_result(result);
        }
    }

    // visit(section, first) for every section, in order
    template <typename Visit>
    void for_each_section(const Visit& visit)
    {
        bool first = true;
        std::apply([&](auto&... section) { ((visit(section, first), first = false), ...); }, m_sections_);
    }

    std::tuple<Sections...> m_sections_;
    static constexpr std::size_t form_capacity = 512;  // a line's names and values (the line is shorter)

    // the console's one task fills them; too big for the task's stack
    std::array<char, form_capacity> m_form_buffer_{};
    FormFields m_fields_{m_form_buffer_};
    SerialConsole m_serial_{this, &handle_line};
};

}  // namespace avela
