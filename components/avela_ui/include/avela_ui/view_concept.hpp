// What every view provides. A view reads the models (const) and hands their values to its skin, which draws them; it
// keeps no state and handles no commands (its controller does). Views are plain classes checked against this concept
// at compile time: no virtual functions, no heap. A view gets its models and its skin through its constructor.
#pragma once

#include <concepts>

namespace avela {

template <typename T>
concept ViewConcept = requires(T& view) {
    { view.draw() } -> std::same_as<void>;     // the whole content area, right after the screen is shown
    { view.refresh() } -> std::same_as<void>;  // every frame: the skin redraws what changed
};

}  // namespace avela
