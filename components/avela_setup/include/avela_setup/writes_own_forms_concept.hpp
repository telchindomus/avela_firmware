// A setup section whose write_form() writes several forms itself (setup_page::write_form_start / write_form_end around
// each), e.g. one per station: its fields would not fit one form (FormFields::capacity).
#pragma once

namespace avela {

template <typename Section>
concept WritesOwnFormsConcept = requires { requires Section::writes_own_forms; };

}  // namespace avela
