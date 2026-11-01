// A controller that has handle() for this command type (handle_command, controller_calls.hpp).
#pragma once

namespace avela {

template <typename T, typename CommandType>
concept HandlesCommandConcept = requires(T& controller, const CommandType& command) { controller.handle(command); };

}  // namespace avela
