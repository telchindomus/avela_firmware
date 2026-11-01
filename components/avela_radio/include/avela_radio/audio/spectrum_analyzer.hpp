// The spectrum of what is playing: the first fft_size samples of a frame (both channels mixed), Hann-windowed, through
// esp-dsp's FFT (the ESP32's float unit), to the RadioModel's levels (spectrum_levels). Its tables are its own
// members; esp-dsp keeps one small table of its own on the heap, made once in init().
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "avela_radio/model/radio_model.hpp"

namespace avela {

class SpectrumAnalyzer {
public:
    static constexpr std::size_t fft_size = 512;  // 86 Hz per bin at 44.1 kHz

    using Levels = std::array<uint8_t, RadioModel::spectrum_band_count>;

    void init();
    // samples: interleaved stereo, at least fft_size frames for a full window (fewer are padded with silence).
    void analyze(std::span<const int16_t> samples, uint32_t sample_rate, Levels& levels);

private:
    std::array<float, fft_size> m_window_{};
    std::array<float, fft_size> m_twiddles_{};             // esp-dsp's coefficient table for fft_size
    std::array<float, fft_size * 2> m_complex_samples_{};  // re, im, re, im, ...
    std::array<float, fft_size / 2> m_bin_power_{};
};

}  // namespace avela
