#include "avela_radio/audio/mp3_decoder.hpp"

#include "esp_check.h"
#include "esp_log.h"
#include "esp_mp3_dec.h"

namespace avela {
namespace {

constexpr const char* log_tag = "mp3";

}  // namespace

//--------------------------------------------------------------------------------------------------

Mp3Decoder::~Mp3Decoder()
{
    if (m_decoder_ != nullptr) {
        esp_audio_simple_dec_close(m_decoder_);
    }
}

//--------------------------------------------------------------------------------------------------

void Mp3Decoder::init()
{
    // registered for the whole program: a second decoder finds it done
    const esp_audio_err_t registered = esp_mp3_dec_register();
    ESP_ERROR_CHECK(registered == ESP_AUDIO_ERR_OK || registered == ESP_AUDIO_ERR_ALREADY_EXIST ? ESP_OK : ESP_FAIL);
    esp_audio_simple_dec_cfg_t config{};
    config.dec_type = ESP_AUDIO_SIMPLE_DEC_TYPE_MP3;
    ESP_ERROR_CHECK(esp_audio_simple_dec_open(&config, &m_decoder_) == ESP_AUDIO_ERR_OK ? ESP_OK : ESP_FAIL);
}

//--------------------------------------------------------------------------------------------------

void Mp3Decoder::reset()
{
    esp_audio_simple_dec_reset(m_decoder_);
}

//--------------------------------------------------------------------------------------------------

Mp3Decoder::Result Mp3Decoder::decode(std::span<uint8_t> input)
{
    esp_audio_simple_dec_raw_t raw{};
    raw.buffer = input.data();
    raw.len = static_cast<uint32_t>(input.size());
    esp_audio_simple_dec_out_t out{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): the codec's C API writes PCM as bytes
    out.buffer = reinterpret_cast<uint8_t*>(m_samples_.data());
    out.len = static_cast<uint32_t>(std::span{m_samples_}.size_bytes());

    const esp_audio_err_t error = esp_audio_simple_dec_process(m_decoder_, &raw, &out);
    if (error != ESP_AUDIO_ERR_OK) {
        // ESP_AUDIO_ERR_BUFF_NOT_ENOUGH can't happen: no MP3 frame is longer than samples_per_channel
        ESP_LOGD(log_tag, "decoding failed (%d)", static_cast<int>(error));
        reset();
        return Result{.m_consumed = input.size(), .m_frame = std::nullopt, .m_failed = true};
    }
    Result result{.m_consumed = raw.consumed, .m_frame = std::nullopt, .m_failed = false};
    if (out.decoded_size == 0) {
        return result;
    }
    esp_audio_simple_dec_info_t info{};
    esp_audio_simple_dec_get_info(m_decoder_, &info);
    std::size_t sample_count = out.decoded_size / sizeof(int16_t);
    if (info.channel == 1) {
        make_stereo(sample_count);
        sample_count *= channels;
    }
    result.m_frame =
        Frame{.m_samples = std::span{m_samples_}.first(sample_count),
              .m_sample_rate = info.sample_rate,
              // kbit/s, though esp_audio_simple_dec.h says bits per second: the MP3 decoder gives the
              // frame header's bitrate as its table has it (esp_audio_codec 2.6.2; its own MP3 test reads
              // that table in kbit/s, and taking the value as bits per second showed 0 kbit/s on the board)
              .m_kilobits_per_second = info.bitrate};
    return result;
}

//--------------------------------------------------------------------------------------------------

// Backwards, so that no sample is overwritten before it is copied.
void Mp3Decoder::make_stereo(std::size_t sample_count)
{
    for (std::size_t index = sample_count; index > 0; --index) {
        const int16_t sample = m_samples_.at(index - 1);
        m_samples_.at((index - 1) * channels) = sample;
        m_samples_.at(((index - 1) * channels) + 1) = sample;
    }
}

}  // namespace avela
