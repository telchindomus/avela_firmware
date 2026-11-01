#include "avela_radio/audio/spectrum_analyzer.hpp"

#include "avela_radio/audio/spectrum_bands.hpp"
#include "dsps_fft2r.h"
#include "dsps_wind_hann.h"
#include "esp_check.h"

namespace avela {
namespace {

constexpr float full_scale = 32768.0f;
// A full-scale sine's FFT magnitude: half of the points' sum (the other half is the mirrored bin), halved again by
// the Hann window's average.
constexpr float full_scale_magnitude = SpectrumAnalyzer::fft_size / 4.0f;
constexpr float full_scale_power = full_scale_magnitude * full_scale_magnitude;

}  // namespace

//--------------------------------------------------------------------------------------------------

void SpectrumAnalyzer::init()
{
    ESP_ERROR_CHECK(dsps_fft2r_init_fc32(m_twiddles_.data(), static_cast<int>(m_twiddles_.size())));
    dsps_wind_hann_f32(m_window_.data(), static_cast<int>(m_window_.size()));
}

//--------------------------------------------------------------------------------------------------

void SpectrumAnalyzer::analyze(std::span<const int16_t> samples, uint32_t sample_rate, Levels& levels)
{
    constexpr std::size_t channels = 2;
    for (std::size_t index = 0; index < fft_size; ++index) {
        const std::size_t left = index * channels;
        const float mixed = left + 1 < samples.size()
                                ? (static_cast<float>(samples[left]) + static_cast<float>(samples[left + 1])) / 2
                                : 0.0f;
        m_complex_samples_.at(index * 2) = mixed / full_scale * m_window_.at(index);
        m_complex_samples_.at((index * 2) + 1) = 0.0f;
    }
    dsps_fft2r_fc32(m_complex_samples_.data(), static_cast<int>(fft_size));
    dsps_bit_rev_fc32(m_complex_samples_.data(), static_cast<int>(fft_size));
    for (std::size_t bin = 0; bin < m_bin_power_.size(); ++bin) {
        const float real = m_complex_samples_.at(bin * 2);
        const float imaginary = m_complex_samples_.at((bin * 2) + 1);
        m_bin_power_.at(bin) = ((real * real) + (imaginary * imaginary)) / full_scale_power;
    }
    spectrum_levels(m_bin_power_, sample_rate, levels);
}

}  // namespace avela
