// A button's short press, reported on release (ButtonStateMachine; after the double-click window if the button has
// one). What it does is up to the receiver.
#pragma once

namespace avela {

class Button;

struct Click {
    const Button* m_sender = nullptr;
};

}  // namespace avela
