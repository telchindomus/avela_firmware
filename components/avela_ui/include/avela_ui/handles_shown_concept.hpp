// A controller with shown(): the request to make when its screen is shown (controller_calls.hpp).
#pragma once

namespace avela {

template <typename T>
concept HandlesShownConcept = requires(T& controller) { controller.shown(); };

}  // namespace avela
