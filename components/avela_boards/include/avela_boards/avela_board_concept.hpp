// An Avela board: a board (BoardHardware) with Avela's controls, a knob with a push switch and a back button, as the
// senders of their input events, and the audio output the radio plays through. The application's input -> command
// table is written for these controls (main/application.cpp, command_for), and the application checks its Board
// against this. A board with other controls or no sound fulfils BoardHardware, not this.
#pragma once

#include <concepts>

#include "avela_boards/board_hardware_concept.hpp"
#include "avela_hal/audio/i2s_output.hpp"
#include "avela_hal/button/button.hpp"
#include "avela_hal/encoder/encoder.hpp"

namespace avela {

template <typename T>
concept AvelaBoardConcept = BoardHardwareConcept<T> && requires(T& board, const T& const_board) {
    { const_board.knob() } -> std::same_as<const Encoder&>;
    { const_board.knob_button() } -> std::same_as<const Button&>;
    { const_board.back_button() } -> std::same_as<const Button&>;
    { board.audio_output() } -> std::same_as<I2sOutput&>;
};

}  // namespace avela
