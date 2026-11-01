// A rotary encoder's rotation (EC11 and alike): counted by the ESP32's PCNT peripheral (hardware quadrature decoding,
// no interrupts), reported as Turn (avela_hal/input/events/) in whole detents (DetentCounter), with itself as sender.
// Its push switch is a GpioButton of its own on EncoderConfig::m_button. Pins and the counts per detent come from the
// board.
#pragma once

#include <cstdint>
#include <optional>

#include "avela_hal/encoder/detent_counter.hpp"
#include "avela_hal/encoder/encoder.hpp"
#include "avela_hal/encoder/encoder_config.hpp"
#include "avela_hal/input/events/input_event.hpp"
#include "avela_hal/input/input_concept.hpp"
#include "driver/pulse_cnt.h"

namespace avela {

class PcntEncoder : public Encoder {
public:
    explicit PcntEncoder(const EncoderConfig& config) : m_config_(config), m_detents_(config.m_counts_per_detent) {}

    void init();
    // A Turn if the knob moved by whole detents since the last poll (the counter counts between polls; the time is
    // not needed).
    std::optional<InputEvent> poll(uint32_t now_ms);

private:
    static constexpr int counter_limit = 1000;          // PCNT unit range: -limit..+limit
    static constexpr int clear_threshold = 800;         // the count is reset before it reaches the limit
    static constexpr uint32_t glitch_filter_ns = 1000;  // pulses shorter than this are contact bounce

    EncoderConfig m_config_;
    DetentCounter m_detents_;
    pcnt_unit_handle_t m_unit_ = nullptr;
    int m_last_count_ = 0;
};

static_assert(InputConcept<PcntEncoder>);

}  // namespace avela
