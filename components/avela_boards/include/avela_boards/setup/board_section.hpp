// The board settings section (setup page and serial console): the screen's brightness, and a restart. Saving stores
// BoardSettings; the app applies them on the main loop. A restart comes a second after the answer, so the page (or
// the console) still gets it. Runs on the web server's or the console's task: reads and writes only Settings.
#pragma once

#include <array>
#include <string_view>

#include "avela_setup/apply_result.hpp"
#include "avela_setup/setup_field.hpp"
#include "avela_setup/setup_section_concept.hpp"

namespace avela {

class BoardSection {
public:
    static constexpr std::string_view id = "board";
    static constexpr std::string_view title = "Board";
    static constexpr bool writes_own_forms = true;  // brightness, and the restart button on its own
    static constexpr std::array fields{
        SetupField{.m_name = "brightness", .m_description = "the screen's brightness, 10..100 (%)", .m_secret = false},
        SetupField{.m_name = "restart", .m_description = "yes: restart the radio", .m_secret = false},
    };

    static void write_form(HtmlWriter& writer);
    static ApplyResult apply(const FormFields& submitted);
    static void write_values(ValueWriter& values);
};

static_assert(SetupSectionConcept<BoardSection>);

}  // namespace avela
