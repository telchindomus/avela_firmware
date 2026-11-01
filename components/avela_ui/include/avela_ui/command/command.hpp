// What a screen's controller receives: a command, made by the application from an input event and its sender
// (avela_hal's input events have no meaning; the application gives them one, the same on every screen). The shown
// screen's controller decides what a command does there (avela_ui/controller_calls.hpp).
#pragma once

#include <variant>

#include "avela_ui/command/back_command.hpp"
#include "avela_ui/command/decrement_command.hpp"
#include "avela_ui/command/increment_command.hpp"
#include "avela_ui/command/menu_command.hpp"
#include "avela_ui/command/ok_command.hpp"
#include "avela_ui/command/options_command.hpp"

namespace avela {

using Command = std::variant<OkCommand, BackCommand, IncrementCommand, DecrementCommand, MenuCommand, OptionsCommand>;

}  // namespace avela
