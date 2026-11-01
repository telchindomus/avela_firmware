// Where a board's I2S amplifier (MAX98357A and alike: I2S in, speaker out) is wired (I2sOutput).
#pragma once

#include "driver/gpio.h"
#include "driver/i2s_types.h"

namespace avela {

struct I2sOutputConfig {
    i2s_port_t m_port;
    gpio_num_t m_bit_clock;    // BCLK
    gpio_num_t m_word_select;  // LRC (left/right clock)
    gpio_num_t m_data_out;     // DIN on the amplifier
};

}  // namespace avela
