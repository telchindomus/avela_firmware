// Every push button, whatever reads it (GpioButton): the sender of a button's input events (Click, DoubleClick,
// LongPress), so a receiver can tell the buttons apart (`click.m_sender == &board.back_button()`). An empty base
// for identity only, no virtual functions: what a button does is InputConcept (avela_hal/input/input_concept.hpp).
// Not copyable: a copy would be another sender.
#pragma once

namespace avela {

class Button {
public:
    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;
    Button(Button&&) = delete;
    Button& operator=(Button&&) = delete;

protected:
    Button() = default;
    ~Button() = default;
};

}  // namespace avela
