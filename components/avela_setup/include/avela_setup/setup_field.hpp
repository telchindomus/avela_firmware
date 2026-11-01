// A field a section takes: its name in the form (and on the console: name=value), what it is, and whether it is a
// secret (a password: not shown, and left out of a settings dump unless asked for).
#pragma once

#include <string_view>

namespace avela {

struct SetupField {
    std::string_view m_name;
    std::string_view m_description;
    bool m_secret = false;
};

}  // namespace avela
