#include "application.hpp"

#include <optional>
#include <variant>

#include "avela_boards/model/board_settings.hpp"
#include "avela_clock/system_clock.hpp"
#include "avela_settings/settings.hpp"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace avela {

namespace {

constexpr uint32_t input_interval_ms = 5;
constexpr uint32_t frame_interval_ms = 33;
constexpr const char* log_tag = "avela";

}  // namespace

//--------------------------------------------------------------------------------------------------

void Application::init()
{
    m_board_.init();
    Settings::init();
    apply_board_settings();
    m_fonts_.load();
    m_radio_storage_.init(m_radio_);
    m_setup_console_.init();
    SystemClock::init();
    m_network_service_.init();
    m_radio_service_.init();
#if CONFIG_AVELA_WEATHER
    m_weather_service_.init();
#endif
    m_setup_server_.init();
    m_firmware_update_service_.init(m_setup_server_.http_server());

    update_models();
    m_ui_.start(m_route_);
    ESP_LOGI(log_tag, "UI started, free internal heap %zu bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
}

//--------------------------------------------------------------------------------------------------

void Application::run()
{
    while (true) {
        const auto uptime = static_cast<uint64_t>(esp_timer_get_time());
        const auto now_ms = static_cast<uint32_t>(uptime / 1000);

        handle_inputs(now_ms);
        if (static_cast<int32_t>(now_ms - m_next_frame_ms_) >= 0) {
            m_next_frame_ms_ = now_ms + frame_interval_ms;
            draw_frame();
        }
        vTaskDelay(pdMS_TO_TICKS(input_interval_ms));
    }
}

//--------------------------------------------------------------------------------------------------

void Application::handle_inputs(uint32_t now_ms)
{
    for (const InputEvent& event : m_board_.poll_inputs(now_ms)) {
        const std::optional<Command> command =
            std::visit([this](const auto& input) { return command_for(input); }, event);
        if (command) {
            m_ui_.handle(*command, m_route_);
        }
    }
}

//--------------------------------------------------------------------------------------------------

void Application::route(const Request& request)
{
    std::visit([this](const auto& alternative) { deliver(alternative); }, request);
}

//--------------------------------------------------------------------------------------------------

void Application::deliver(const PlayerRequest& request)
{
    if (!m_radio_service_.queues().requests().send(request)) {
        ESP_LOGW(log_tag, "radio busy: a request dropped");
    }
}

//--------------------------------------------------------------------------------------------------

void Application::deliver(const SetupNetworkRequest& request)
{
    if (!m_network_service_.requests().send(request)) {
        ESP_LOGW(log_tag, "network busy: a request dropped");
    }
}

//--------------------------------------------------------------------------------------------------

void Application::deliver(const AcceptUploadsRequest& request)
{
    if (!m_firmware_update_service_.requests().send(request)) {
        ESP_LOGW(log_tag, "firmware update busy: a request dropped");
    }
}

#if CONFIG_AVELA_WEATHER
//--------------------------------------------------------------------------------------------------

void Application::deliver(const WeatherUpdateRequest& request)
{
    if (!m_weather_service_.requests().send(request)) {
        ESP_LOGW(log_tag, "weather busy: a request dropped");
    }
}
#endif

//--------------------------------------------------------------------------------------------------

std::optional<Command> Application::command_for(const Turn& turn) const
{
    if (turn.m_sender != &m_board_.knob()) {
        return std::nullopt;
    }
    if (turn.m_detents > 0) {
        return IncrementCommand{.m_steps = turn.m_detents};
    }
    if (turn.m_detents < 0) {
        return DecrementCommand{.m_steps = -turn.m_detents};
    }
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------

std::optional<Command> Application::command_for(const Click& click) const
{
    if (click.m_sender == &m_board_.knob_button()) {
        return OkCommand{};
    }
    if (click.m_sender == &m_board_.back_button()) {
        return BackCommand{};
    }
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------

std::optional<Command> Application::command_for(const DoubleClick& double_click) const
{
    if (double_click.m_sender == &m_board_.knob_button()) {
        return MenuCommand{};
    }
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------

std::optional<Command> Application::command_for(const LongPress& long_press) const
{
    if (long_press.m_sender == &m_board_.knob_button()) {
        return OptionsCommand{};
    }
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------

void Application::apply_board_settings()
{
    m_board_settings_revision_ = Settings::revision<BoardSettings>();  // before loading: a save in between comes next
    BoardSettings settings{};
    if (!Settings::load(settings)) {
        settings = BoardSettings{};
    }
    m_board_.display().setBrightness(backlight_level(settings.m_brightness_percent));
}

//--------------------------------------------------------------------------------------------------

void Application::update_models()
{
    SystemClock::update();
    if (Settings::revision<BoardSettings>() != m_board_settings_revision_) {
        apply_board_settings();
    }
    m_radio_storage_.update(m_radio_);  // the stations first: the radio service follows them
    m_radio_service_.update(m_radio_);
    m_system_info_service_.update(m_system_info_);
    m_network_service_.update(m_network_);
    m_firmware_update_service_.update(m_firmware_update_);
#if CONFIG_AVELA_WEATHER
    m_weather_service_.update(m_weather_);
#endif
}

//--------------------------------------------------------------------------------------------------

void Application::draw_frame()
{
    update_models();
    m_ui_.refresh(m_route_);
}

}  // namespace avela
