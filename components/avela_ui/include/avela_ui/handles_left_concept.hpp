// A controller with left(): the request to make when its screen is left (controller_calls.hpp).
#pragma once

namespace avela {

template <typename T>
concept HandlesLeftConcept = requires(T& controller) { controller.left(); };

}  // namespace avela
