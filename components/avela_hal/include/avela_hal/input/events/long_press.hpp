// A button held for its long-press time, reported once while still held (ButtonStateMachine). What it does is up to
// the receiver.
#pragma once

namespace avela {

class Button;

struct LongPress {
    const Button* m_sender = nullptr;
};

}  // namespace avela
