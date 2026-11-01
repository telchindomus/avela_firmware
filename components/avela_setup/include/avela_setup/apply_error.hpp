// Why apply() didn't save, for programs (the serial console's machine commands, the avela CLI); people read the
// message.
#pragma once

#include <cstdint>

namespace avela {

enum class ApplyError : uint8_t {
    none,
    invalid_value,   // m_field's value breaks a rule (the message says which)
    storage_failed,  // NVS refused it
    unknown_field,   // no field of that name (the console checks names before apply())
};

}  // namespace avela
