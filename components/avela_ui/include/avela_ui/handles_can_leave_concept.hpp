// A controller with can_leave(): false while its screen must stay (controller_calls.hpp).
#pragma once

#include <concepts>

namespace avela {

template <typename T>
concept HandlesCanLeaveConcept = requires(const T& controller) {
    { controller.can_leave() } -> std::same_as<bool>;
};

}  // namespace avela
