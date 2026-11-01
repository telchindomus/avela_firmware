// How the Ui talks to a controller (controller_concept.hpp): gives it a command (its handle() for that command type,
// if it has one, decided at compile time), asks for its request when its screen is shown or left, and whether its
// screen may be left now; each hook only where the controller has it.
#pragma once

#include <concepts>
#include <optional>
#include <variant>

#include "avela_ui/command/command.hpp"
#include "avela_ui/controller_concept.hpp"
#include "avela_ui/handles_can_leave_concept.hpp"
#include "avela_ui/handles_command_concept.hpp"
#include "avela_ui/handles_left_concept.hpp"
#include "avela_ui/handles_shown_concept.hpp"
#include "avela_ui/response.hpp"

namespace avela {

// Gives one command to the controller's handle() for its type, if it has one.
template <ControllerConcept T, typename CommandType>
Response handle_one(T& controller, const CommandType& command)
{
    if constexpr (HandlesCommandConcept<T, CommandType>) {
        static_assert(std::same_as<decltype(controller.handle(command)), Response>,
                      "a controller's handle() returns a Response");
        return controller.handle(command);
    } else {
        return {};
    }
}

// Gives the command to the controller's handle() for its type, if it has one.
template <ControllerConcept T>
Response handle_command(T& controller, const Command& command)
{
    return std::visit([&controller](const auto& alternative) { return handle_one(controller, alternative); }, command);
}

// The request the controller makes when its screen is shown, if any.
template <ControllerConcept T>
std::optional<Request> shown_request(T& controller)
{
    if constexpr (HandlesShownConcept<T>) {
        static_assert(std::same_as<decltype(controller.shown()), Request>, "a controller's shown() returns a Request");
        return controller.shown();
    } else {
        return std::nullopt;
    }
}

// The request the controller makes when its screen is left, if any.
template <ControllerConcept T>
std::optional<Request> left_request(T& controller)
{
    if constexpr (HandlesLeftConcept<T>) {
        static_assert(std::same_as<decltype(controller.left()), Request>, "a controller's left() returns a Request");
        return controller.left();
    } else {
        return std::nullopt;
    }
}

// Whether the controller's screen may be left now.
template <ControllerConcept T>
bool may_leave(const T& controller)
{
    if constexpr (HandlesCanLeaveConcept<T>) {
        return controller.can_leave();
    } else {
        return true;
    }
}

}  // namespace avela
