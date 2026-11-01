// Avela Piccolo: the CYD 1.9 module extended with what the enclosure adds around it: the EC11 knob (its encoder and
// its push switch), the MAX98357A amplifier (I2S) with its speaker (passive, nothing to configure), and the roles of
// the module's parts (its BOOT button is the back button). The board reports what its inputs did, with their
// senders, and gives no meaning: that is the application's. Picked in menuconfig (Avela board), this header names
// the build's `Board`.
#pragma once

#include <cstdint>
#include <span>

#include "avela_boards/avela_board_concept.hpp"
#include "avela_boards/cyd_1_9.hpp"
#include "avela_hal/audio/i2s_output.hpp"
#include "avela_hal/audio/i2s_output_config.hpp"
#include "avela_hal/button/button.hpp"
#include "avela_hal/button/button_timing.hpp"
#include "avela_hal/button/gpio_button.hpp"
#include "avela_hal/encoder/encoder.hpp"
#include "avela_hal/encoder/encoder_config.hpp"
#include "avela_hal/encoder/pcnt_encoder.hpp"
#include "avela_hal/input/events/input_event.hpp"
#include "avela_hal/input/input_poller.hpp"
#include "driver/gpio.h"

namespace avela {

class AvelaPiccolo : public Cyd_1_9 {
public:
    static constexpr EncoderConfig knob_config{
        .m_phase_a = GPIO_NUM_22,  // module pin S1
        .m_phase_b = GPIO_NUM_23,  // module pin S2
        .m_button = GPIO_NUM_4,    // module pin KEY; the module's power pin on 3V3 (never 5V)
        .m_counts_per_detent = 4,  // EC11, full quadrature (measured with the pin test)
    };

    static constexpr I2sOutputConfig amplifier{
        .m_port = I2S_NUM_0,
        .m_bit_clock = GPIO_NUM_26,    // MAX98357A BCLK
        .m_word_select = GPIO_NUM_25,  // MAX98357A LRC
        .m_data_out = GPIO_NUM_27,     // MAX98357A DIN
    };

    static constexpr uint8_t display_rotation = 3;

    AvelaPiccolo() : Cyd_1_9{back_button_timing} {}

    // Once: the module (display, BOOT button), the display's rotation, the knob and the amplifier.
    void init();

    // The amplifier, for the radio to play through; set up by init().
    I2sOutput& audio_output() { return m_amplifier_; }

    // What the inputs did since the last poll, in input order, with their senders; valid until the next poll.
    std::span<const InputEvent> poll_inputs(uint32_t now_ms) { return m_inputs_.poll(now_ms); }

    // The inputs as senders: compare with an event's m_sender.
    [[nodiscard]] const Encoder& knob() const { return m_knob_; }
    [[nodiscard]] const Button& knob_button() const { return m_knob_button_; }
    [[nodiscard]] const Button& back_button() const { return boot_button(); }

private:
    PcntEncoder m_knob_{knob_config};
    GpioButton m_knob_button_{knob_config.m_button, default_button_timing};
    I2sOutput m_amplifier_{amplifier};
    InputPoller<PcntEncoder, GpioButton, GpioButton> m_inputs_{m_knob_, m_knob_button_, boot_button_input()};
};

static_assert(AvelaBoardConcept<AvelaPiccolo>);

using Board = AvelaPiccolo;

}  // namespace avela
