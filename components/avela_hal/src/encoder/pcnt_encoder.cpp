#include "avela_hal/encoder/pcnt_encoder.hpp"

#include <initializer_list>

#include "esp_check.h"

namespace avela {

//--------------------------------------------------------------------------------------------------

void PcntEncoder::init()
{
    pcnt_unit_config_t unit_config{};
    unit_config.low_limit = -counter_limit;
    unit_config.high_limit = counter_limit;
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &m_unit_));

    pcnt_glitch_filter_config_t filter_config{};
    filter_config.max_glitch_ns = glitch_filter_ns;
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(m_unit_, &filter_config));

    // Standard x4 quadrature decoding: each phase counts on its own edges, direction from the other phase's level.
    pcnt_chan_config_t channel_a_config{};
    channel_a_config.edge_gpio_num = m_config_.m_phase_a;
    channel_a_config.level_gpio_num = m_config_.m_phase_b;
    pcnt_channel_handle_t channel_a = nullptr;
    ESP_ERROR_CHECK(pcnt_new_channel(m_unit_, &channel_a_config, &channel_a));

    pcnt_chan_config_t channel_b_config{};
    channel_b_config.edge_gpio_num = m_config_.m_phase_b;
    channel_b_config.level_gpio_num = m_config_.m_phase_a;
    pcnt_channel_handle_t channel_b = nullptr;
    ESP_ERROR_CHECK(pcnt_new_channel(m_unit_, &channel_b_config, &channel_b));

    ESP_ERROR_CHECK(
        pcnt_channel_set_edge_action(channel_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(
        pcnt_channel_set_level_action(channel_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(
        pcnt_channel_set_edge_action(channel_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(
        pcnt_channel_set_level_action(channel_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    // The ESP32's pull-ups on the phases, in addition to the module's own (its power pin must be on 3V3: left
    // floating, those tie KEY to S1/S2 and turning reads as button presses).
    for (const gpio_num_t pin : {m_config_.m_phase_a, m_config_.m_phase_b}) {
        ESP_ERROR_CHECK(gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY));
    }

    ESP_ERROR_CHECK(pcnt_unit_enable(m_unit_));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(m_unit_));
    ESP_ERROR_CHECK(pcnt_unit_start(m_unit_));
}

//--------------------------------------------------------------------------------------------------

std::optional<InputEvent> PcntEncoder::poll(uint32_t /*now_ms*/)
{
    int count = 0;
    ESP_ERROR_CHECK(pcnt_unit_get_count(m_unit_, &count));
    const int counts = count - m_last_count_;
    m_last_count_ = count;
    if (count > clear_threshold || count < -clear_threshold) {
        ESP_ERROR_CHECK(pcnt_unit_clear_count(m_unit_));
        m_last_count_ = 0;
    }
    const int detents = m_detents_.add(counts);
    if (detents == 0) {
        return std::nullopt;
    }
    return Turn{.m_sender = this, .m_detents = detents};
}

}  // namespace avela
