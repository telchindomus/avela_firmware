// What every controller provides. A controller gives the commands their meaning on its screen: for each command it
// handles (`handle(XCommand)`) it returns a Response: the screen to open or its own to close, and/or a request.
// It keeps no state: what it changes is its page model, if its screen has one; the data models it reads only.
// Commands it has no handle() for are ignored, decided at compile time (handle_command). BackCommand never reaches a
// controller: Back closes the shown screen, back to the one that opened it, for every screen (the Ui).
// A controller may also have `shown()`: the request to make when its screen is shown (the Wi-Fi setup screen asks
// for the setup network), `left()`: the request to make when it is left (closing it again), and `can_leave()`:
// false while its screen must stay (the firmware update while an image is written).
#pragma once

#include <type_traits>

namespace avela {

// Any class: what it handles is found at compile time (its handle() overloads and hooks).
template <typename T>
concept ControllerConcept = std::is_class_v<T>;

}  // namespace avela
