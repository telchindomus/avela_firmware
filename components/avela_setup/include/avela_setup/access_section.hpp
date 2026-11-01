// The Access settings section (setup page and serial console): the page password (page_password.hpp). Write-only:
// the tab and the console say whether one is set, never more, and a settings dump leaves it out, so a restore keeps
// the radio's own. Saving stores the new password's hash: the page asks for it from the next request on. Runs on the
// web server's or the console's task: reads and writes only Settings.
#pragma once

#include <array>
#include <string_view>

#include "avela_setup/apply_result.hpp"
#include "avela_setup/setup_field.hpp"
#include "avela_setup/setup_section_concept.hpp"

namespace avela {

class AccessSection {
public:
    static constexpr std::string_view id = "access";
    static constexpr std::string_view title = "Access";
    static constexpr std::array fields{
        SetupField{.m_name = "password",
                   .m_description = "the setup page's password on the home network (8 to 32 characters)",
                   .m_secret = true},
    };

    static void write_form(HtmlWriter& writer);
    static ApplyResult apply(const FormFields& submitted);
    static void write_values(ValueWriter& values);
};

static_assert(SetupSectionConcept<AccessSection>);

}  // namespace avela
