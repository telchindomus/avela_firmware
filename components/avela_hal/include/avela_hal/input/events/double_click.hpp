// Two presses of a button within its double-click window, reported on the second release (ButtonStateMachine).
// What it does is up to the receiver.
#pragma once

namespace avela {

class Button;

struct DoubleClick {
    const Button* m_sender = nullptr;
};

}  // namespace avela
