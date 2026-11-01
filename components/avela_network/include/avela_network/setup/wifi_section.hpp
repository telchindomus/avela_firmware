// The Wi-Fi settings section (setup page and serial console): the home network's name and its password; the radio's
// host name. Saving the network stores
// WifiCredentials, and the network service sees the new revision and joins it; saving the host name stores HostName,
// in force from the next start. On the page they are two forms (the network form always sends its password); on the
// setup network, while no page password is set, a reminder to set one first (Access tab).
// Runs on the web server's or the console's task: reads only Settings and the model's host name (safe from any task:
// it doesn't change after the start).
#pragma once

#include <array>
#include <string_view>

#include "avela_network/model/network_model.hpp"
#include "avela_setup/apply_result.hpp"
#include "avela_setup/setup_field.hpp"
#include "avela_setup/setup_section_concept.hpp"

namespace avela {

class WifiSection {
public:
    static constexpr std::string_view id = "wifi";
    static constexpr std::string_view title = "Wi-Fi";
    static constexpr bool writes_own_forms = true;  // the network, and the host name on its own
    static constexpr std::array fields{
        SetupField{.m_name = "ssid", .m_description = "the home network's name", .m_secret = false},
        SetupField{.m_name = "password",
                   .m_description = "its password (8 to 63 characters; empty: an open network)",
                   .m_secret = true},
        SetupField{.m_name = "host_name",
                   .m_description = "the radio's name: http://<name>.local (a-z, 0-9, '-'; from the next start)",
                   .m_secret = false},
    };

    explicit WifiSection(const NetworkModel& model) : m_model_(model) {}

    void write_form(HtmlWriter& writer);
    [[nodiscard]] ApplyResult apply(const FormFields& submitted) const;
    static void write_values(ValueWriter& values);

private:
    // The network part: ssid and password (a field not given keeps its saved value).
    [[nodiscard]] ApplyResult apply_network(const FormFields& submitted) const;

    const NetworkModel& m_model_;
};

static_assert(SetupSectionConcept<WifiSection>);

}  // namespace avela
