#include "avela_network/setup/wifi_section.hpp"

#include <optional>

#include "avela_settings/settings.hpp"
#include "avela_setup/access_section.hpp"
#include "avela_setup/setup_server.hpp"

namespace avela {

//--------------------------------------------------------------------------------------------------

void WifiSection::write_form(HtmlWriter& writer)
{
    WifiCredentials saved{};
    const bool has_saved = Settings::load(saved);

    if (setup_page::setup_network_up() && !setup_page::page_password_set()) {
        // saving the network closes the setup network, and without a password the page needs it
        writer.raw(R"(<p>First set a page password on the <a href="/?tab=)").text(AccessSection::id).raw(R"(">)");
        writer.text(AccessSection::title)
            .raw("</a> tab: once the radio has joined your network, this setup network "
                 "closes, and on your network the page opens only with it.</p>");
    }
    setup_page::write_form_start(writer, id);
    writer.raw(R"(<label for="ssid">Network</label>)"
               R"(<input id="ssid" name="ssid" maxlength="32" required )"
               R"(autocapitalize="none" autocorrect="off" value=")");
    if (has_saved) {
        writer.text(saved.m_ssid.view());
    }
    writer.raw(R"(">)");
    writer.raw(R"(<label for="password">Password</label>)"
               R"(<input id="password" name="password" type="password" maxlength="63" )"
               R"(placeholder="empty for an open network">)");
    setup_page::write_form_end(writer);

    const HostName host_name = saved_host_name();
    setup_page::write_form_start(writer, id);
    writer.raw(
        R"(<label for="host_name">Radio name</label>)"
        R"(<input id="host_name" name="host_name" maxlength="31" required pattern="[a-z0-9]([a-z0-9\-]*[a-z0-9])?" )"
        R"(autocapitalize="none" autocorrect="off" value=")");
    writer.text(host_name.m_name.view()).raw(R"(">)");
    if (host_name.m_name.view() != m_model_.m_host_name.view()) {
        writer.raw("<p>Now http://").text(m_model_.m_host_name.view()).raw(".local; http://");
        writer.text(host_name.m_name.view()).raw(".local after a restart (Board).</p>");
    } else {
        writer.raw("<p>The page is at http://").text(host_name.m_name.view()).raw(".local</p>");
    }
    setup_page::write_form_end(writer);
}

//--------------------------------------------------------------------------------------------------

void WifiSection::write_values(ValueWriter& values)
{
    WifiCredentials saved{};
    if (!Settings::load(saved)) {
        saved = WifiCredentials{};
        values.note("no network saved");
    }
    values.value("ssid", saved.m_ssid.view());
    values.secret("password", saved.m_password.view());
    values.value("host_name", saved_host_name().m_name.view());
}

//--------------------------------------------------------------------------------------------------

ApplyResult WifiSection::apply(const FormFields& submitted) const
{
    const std::optional<std::string_view> host_name = submitted.find("host_name");
    const bool network_given = submitted.find("ssid") || submitted.find("password");
    if (!host_name && !network_given) {
        return ApplyResult::invalid("", "Nothing to do: give ssid, password or host_name.");
    }
    if (host_name) {
        if (!valid_host_name(*host_name)) {
            return ApplyResult::invalid("host_name",
                                        "A name has 1 to 31 characters: a-z, 0-9 and '-' (not first or last).");
        }
        HostName saved{};
        saved.m_name.append(*host_name);
        if (!Settings::save(saved)) {
            return ApplyResult::storage_failed();
        }
    }
    if (!network_given) {
        ApplyResult result;
        result.m_saved = true;
        result.m_message.append("Saved: the radio is http://")
            .append(*host_name)
            .append(".local from the next start (Board: Restart).");
        return result;
    }
    return apply_network(submitted);
}

//--------------------------------------------------------------------------------------------------

ApplyResult WifiSection::apply_network(const FormFields& submitted) const
{
    // a field that isn't given keeps its saved value (the console may change one at a time)
    WifiCredentials saved{};
    if (!Settings::load(saved)) {
        saved = WifiCredentials{};
    }
    const std::string_view ssid = submitted.find("ssid").value_or(saved.m_ssid.view());
    const std::string_view password = submitted.find("password").value_or(saved.m_password.view());
    if (ssid.empty() || ssid.size() > WifiCredentials::ssid_capacity - 1) {
        return ApplyResult::invalid("ssid", "The network name needs 1 to 32 characters.");
    }
    if (!password.empty() && (password.size() < WifiCredentials::minimum_password_length ||
                              password.size() > WifiCredentials::password_capacity - 1)) {
        return ApplyResult::invalid("password",
                                    "A Wi-Fi password has 8 to 63 characters (or none for an open network).");
    }
    WifiCredentials credentials{};
    credentials.m_ssid.append(ssid);
    credentials.m_password.append(password);
    if (!Settings::save(credentials)) {
        return ApplyResult::storage_failed();
    }
    ApplyResult result;
    result.m_saved = true;
    result.m_message.append("Avela is joining ")
        .append(ssid)
        .append(". Once it has joined, this setup network closes; the page is then at http://")
        .append(m_model_.m_host_name.view())
        .append(".local on your network.");
    return result;
}

}  // namespace avela
