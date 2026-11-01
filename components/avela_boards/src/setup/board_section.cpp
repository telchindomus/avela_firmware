#include "avela_boards/setup/board_section.hpp"

#include <optional>

#include "avela_boards/model/board_settings.hpp"
#include "avela_settings/settings.hpp"
#include "avela_setup/setup_server.hpp"
#include "avela_setup/whole_number.hpp"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

namespace avela {
namespace {

using Number = FixedString<number_capacity<int>>;

constexpr TickType_t restart_delay = pdMS_TO_TICKS(1000);  // the answer goes out first

BoardSettings saved_settings()
{
    BoardSettings settings{};
    if (!Settings::load(settings)) {
        settings = BoardSettings{};
    }
    return settings;
}

void restart_soon()
{
    static StaticTimer_t timer_storage;
    static TimerHandle_t timer = xTimerCreateStatic(
        "restart", restart_delay, pdFALSE, nullptr, [](TimerHandle_t /*timer*/) { esp_restart(); }, &timer_storage);
    xTimerStart(timer, 0);
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void BoardSection::write_form(HtmlWriter& writer)
{
    Number brightness;
    brightness.append_number(saved_settings().m_brightness_percent);
    setup_page::write_form_start(writer, id);
    writer.raw(R"(<label for="brightness">Screen brightness (%)</label>)"
               R"(<input id="brightness" name="brightness" type="number" min="10" max="100" value=")");
    writer.text(brightness.view()).raw(R"(">)");
    setup_page::write_form_end(writer);
    setup_page::write_form_start(writer, id);
    writer.raw(R"(<input type="hidden" name="restart" value="yes"><button type="submit">Restart</button></form>)");
}

//--------------------------------------------------------------------------------------------------

void BoardSection::write_values(ValueWriter& values)
{
    Number brightness;
    brightness.append_number(saved_settings().m_brightness_percent);
    values.value("brightness", brightness.view());
    values.value("restart", "no");
}

//--------------------------------------------------------------------------------------------------

ApplyResult BoardSection::apply(const FormFields& submitted)
{
    const std::optional<std::string_view> brightness = submitted.find("brightness");
    const std::optional<std::string_view> restart = submitted.find("restart");
    if (restart && *restart != "yes" && *restart != "no") {
        return ApplyResult::invalid("restart", "Restart is yes or no.");
    }
    const bool restarting = restart == "yes";
    if (!brightness && !restarting) {
        return ApplyResult::invalid("", "Nothing to do: give brightness or restart=yes.");
    }
    FixedString<ApplyResult::message_capacity> done;
    if (brightness) {
        const std::optional<int> percent = whole_number(*brightness);
        if (!percent || *percent < BoardSettings::brightness_min || *percent > BoardSettings::brightness_max) {
            return ApplyResult::invalid("brightness", "The brightness is a whole number, 10..100.");
        }
        BoardSettings settings = saved_settings();
        settings.m_brightness_percent = static_cast<uint8_t>(*percent);
        if (!Settings::save(settings)) {
            return ApplyResult::storage_failed();
        }
        done.append("Brightness ").append_number(*percent).append(" %.");
    }
    if (restarting) {
        restart_soon();
        done.append(done.empty() ? "" : " ").append("Restarting: the page is back in about 10 seconds.");
    }
    ApplyResult result;
    result.m_saved = true;
    result.m_message.append(done.view());
    return result;
}

}  // namespace avela
