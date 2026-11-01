// Where a board's rotary encoder is wired and how many quadrature counts make one detent (PcntEncoder).
#pragma once

#include "driver/gpio.h"

namespace avela {

struct EncoderConfig {
    gpio_num_t m_phase_a;
    gpio_num_t m_phase_b;
    gpio_num_t m_button;      // its push switch (a GpioButton)
    int m_counts_per_detent;  // EC11: 4 (full quadrature)
};

}  // namespace avela
