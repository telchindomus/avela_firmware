#include "avela_hal/audio/i2s_output.hpp"

#include "esp_check.h"
#include "freertos/FreeRTOS.h"

namespace avela {

//--------------------------------------------------------------------------------------------------

void I2sOutput::init()
{
    i2s_chan_config_t channel_config = I2S_CHANNEL_DEFAULT_CONFIG(m_config_.m_port, I2S_ROLE_MASTER);
    channel_config.dma_desc_num = dma_buffer_count;
    channel_config.dma_frame_num = dma_frames_per_buffer;
    // a sent buffer is cleared, so one nothing new was written to plays silence (before or after the on_sent callback
    // is the same here: there is none)
    channel_config.auto_clear_before_cb = true;
    ESP_ERROR_CHECK(i2s_new_channel(&channel_config, &m_channel_, nullptr));

    i2s_std_config_t standard_config{};
    standard_config.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(m_sample_rate_);
    standard_config.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
    standard_config.gpio_cfg.mclk = I2S_GPIO_UNUSED;
    standard_config.gpio_cfg.bclk = m_config_.m_bit_clock;
    standard_config.gpio_cfg.ws = m_config_.m_word_select;
    standard_config.gpio_cfg.dout = m_config_.m_data_out;
    standard_config.gpio_cfg.din = I2S_GPIO_UNUSED;
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(m_channel_, &standard_config));
}

//--------------------------------------------------------------------------------------------------

void I2sOutput::set_sample_rate(uint32_t sample_rate)
{
    if (sample_rate == m_sample_rate_) {
        return;
    }
    // the driver changes the clock only while the channel is stopped; the next write() starts it again
    stop();
    const i2s_std_clk_config_t clock_config = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate);
    ESP_ERROR_CHECK(i2s_channel_reconfig_std_clock(m_channel_, &clock_config));
    m_sample_rate_ = sample_rate;
}

//--------------------------------------------------------------------------------------------------

void I2sOutput::write(std::span<const int16_t> samples)
{
    start();
    std::size_t written = 0;
    ESP_ERROR_CHECK(i2s_channel_write(m_channel_, samples.data(), samples.size_bytes(), &written, portMAX_DELAY));
}

//--------------------------------------------------------------------------------------------------

void I2sOutput::stop()
{
    if (m_running_) {
        ESP_ERROR_CHECK(i2s_channel_disable(m_channel_));
        m_running_ = false;
    }
}

//--------------------------------------------------------------------------------------------------

void I2sOutput::start()
{
    if (!m_running_) {
        ESP_ERROR_CHECK(i2s_channel_enable(m_channel_));
        m_running_ = true;
    }
}

}  // namespace avela
