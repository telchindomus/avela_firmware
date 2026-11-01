// MP3 to PCM, with Espressif's decoder (esp_audio_codec's simple decoder: it finds the frames in bytes cut anywhere).
// decode() takes what was received, uses as much as it needs for one frame and gives the frame as 16-bit stereo
// samples (a mono stream on both channels). The library allocates the decoder's state once, in init().
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include "esp_audio_simple_dec.h"

namespace avela {

class Mp3Decoder {
public:
    static constexpr std::size_t samples_per_channel = 1152;  // per MPEG-1 Layer III frame (the most of any MP3)
    static constexpr std::size_t channels = 2;

    struct Frame {
        std::span<int16_t> m_samples;  // left, right, left, ...; valid until the next decode()
        uint32_t m_sample_rate;
        uint32_t m_kilobits_per_second;  // the frame header's bitrate; 0 if the decoder doesn't know it
    };

    struct Result {
        std::size_t m_consumed = 0;    // bytes of the input used; the rest is for the next decode()
        std::optional<Frame> m_frame;  // when a whole frame was decoded
        bool m_failed = false;         // the bytes aren't MP3 (or are damaged); the next decode() starts afresh
    };

    Mp3Decoder() = default;
    Mp3Decoder(const Mp3Decoder&) = delete;
    Mp3Decoder& operator=(const Mp3Decoder&) = delete;
    Mp3Decoder(Mp3Decoder&&) = delete;
    Mp3Decoder& operator=(Mp3Decoder&&) = delete;
    ~Mp3Decoder();

    void init();
    // For a new stream: forgets the frame it was in the middle of.
    void reset();
    // input: the codec's API takes it as writable, but leaves it as it is.
    Result decode(std::span<uint8_t> input);

private:
    // Spreads a mono frame (the first sample_count samples) over both channels, in place.
    void make_stereo(std::size_t sample_count);

    esp_audio_simple_dec_handle_t m_decoder_ = nullptr;
    std::array<int16_t, samples_per_channel * channels> m_samples_{};
};

}  // namespace avela
