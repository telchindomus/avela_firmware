#include "avela_setup/access_section.hpp"

#include <array>
#include <optional>

#include "avela_settings/settings.hpp"
#include "avela_setup/page_password.hpp"
#include "avela_setup/setup_server.hpp"
#include "esp_random.h"

namespace avela {

//--------------------------------------------------------------------------------------------------

void AccessSection::write_form(HtmlWriter& writer)
{
    if (setup_page::page_password_set()) {
        writer.raw("<p>A password is set: on your home network the page asks for it, with the user name avela-admin. "
                   "Forgotten: set "
                   "a new one here, on the radio's setup network.</p>");
    } else {
        writer.raw("<p>No password yet: until one is set, the page opens only on the radio's setup network.</p>");
    }
    writer.raw(R"(<label for="password">New password</label>)"
               R"(<input id="password" name="password" type="password" minlength="8" maxlength="32" required )"
               R"(autocomplete="new-password">)");
}

//--------------------------------------------------------------------------------------------------

void AccessSection::write_values(ValueWriter& values)
{
    // never the password or its hash: a hash can be guessed against offline
    values.note(setup_page::page_password_set() ? "a page password is set" : "no page password yet");
}

//--------------------------------------------------------------------------------------------------

ApplyResult AccessSection::apply(const FormFields& submitted)
{
    const std::optional<std::string_view> password = submitted.find("password");
    if (!password) {
        return ApplyResult::invalid("", "Nothing to do: give password.");
    }
    if (!page_password::valid(*password)) {
        return ApplyResult::invalid("password", "A page password has 8 to 32 characters: letters, digits, spaces "
                                                "and the signs on a keyboard.");
    }
    std::array<uint8_t, PagePassword::salt_bytes> salt{};
    esp_fill_random(salt.data(), salt.size());  // truly random: the radio is on by the time anyone types here
    const std::optional<PagePassword> record = page_password::hashed(*password, salt);
    if (!record || !Settings::save(*record)) {
        return ApplyResult::storage_failed();
    }
    ApplyResult result;
    result.m_saved = true;
    result.m_message.append("Saved: on your home network the page asks for this password from now on.");
    return result;
}

}  // namespace avela
