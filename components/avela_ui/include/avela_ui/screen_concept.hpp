// What every screen provides. A screen is a page you navigate to (ScreenId): its view over the skin's page skin, its
// controller and, where it has state of its own, its page model, built together when it is shown and gone when it is
// left. Each component provides its screens as templates over the page skin (`PlayerScreen<PageSkin>`); main picks
// the skin and hands them the models.
#pragma once

#include <concepts>

#include "avela_ui/controller_concept.hpp"
#include "avela_ui/screen_id.hpp"
#include "avela_ui/view_concept.hpp"

namespace avela {

template <typename T>
concept ScreenConcept = requires {
    { T::id } -> std::convertible_to<ScreenId>;
    requires ViewConcept<decltype(T::m_view)>;
    requires ControllerConcept<decltype(T::m_controller)>;
};

}  // namespace avela
