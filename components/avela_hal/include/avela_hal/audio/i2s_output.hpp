// Sound out through an I2S amplifier: 16-bit stereo samples (left, right, left, ...) at the stream's sample rate, in
// the I2S standard (Philips) format the MAX98357A takes. write() blocks until the samples are in the driver's DMA
// buffers, which paces the caller to the sample rate. When nothing is written, the DMA buffers play silence (no
// repeated last buffer). The driver allocates its DMA buffers once, in init().
//
// The clocks run only while there is sound: they start with the first write() and stop with stop(). Without the bit
// clock the MAX98357A shuts down by itself (no idle hiss, less current), and it starts again with the clock.
#pragma once

#include <cstdint>
#include <span>

#include "avela_hal/audio/i2s_output_config.hpp"
#include "driver/i2s_std.h"

namespace avela {

class I2sOutput {
public:
    explicit I2sOutput(const I2sOutputConfig& config) : m_config_(config) {}

    // The channel and its DMA buffers; the clocks stay stopped until the first write().
    void init();
    // Changes the clock when the stream's sample rate differs from the current one (stops it; write() starts it).
    void set_sample_rate(uint32_t sample_rate);
    // Interleaved stereo samples; starts the clocks if they are stopped, returns once all of them are queued for
    // playing.
    void write(std::span<const int16_t> samples);
    // Stops the clocks (the amplifier shuts down). Only once the written samples have played: what is still in the
    // DMA buffers is cut off.
    void stop();

private:
    void start();

    static constexpr uint32_t initial_sample_rate = 44'100;
    // DMA buffers: 6 x 240 stereo frames, about 33 ms of sound at 44.1 kHz (5.6 KB)
    static constexpr uint32_t dma_buffer_count = 6;
    static constexpr uint32_t dma_frames_per_buffer = 240;

    I2sOutputConfig m_config_;
    i2s_chan_handle_t m_channel_ = nullptr;
    uint32_t m_sample_rate_ = initial_sample_rate;
    bool m_running_ = false;
};

}  // namespace avela
