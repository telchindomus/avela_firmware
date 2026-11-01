#include "avela_system_info/service/esp_system_info_service.hpp"

#include <chrono>
#include <cstdint>
#include <iterator>
#include <string_view>

#include "avela_strings/fixed_string.hpp"
#include "esp_app_desc.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace avela {
namespace {

constexpr const char* log_tag = "system";
constexpr std::size_t stacks_line_capacity = 384;

}  // namespace

//--------------------------------------------------------------------------------------------------

void EspSystemInfoService::update(SystemInfoModel& model)
{
    namespace chrono = std::chrono;
    constexpr std::size_t bytes_per_kilobyte = 1024;

    model.m_free_kilobytes = heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / bytes_per_kilobyte;
    model.m_minimum_free_kilobytes = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL) / bytes_per_kilobyte;
    model.m_total_kilobytes = heap_caps_get_total_size(MALLOC_CAP_INTERNAL) / bytes_per_kilobyte;
    model.m_firmware_version = std::data(esp_app_get_description()->version);
    model.m_uptime = chrono::duration_cast<chrono::seconds>(chrono::microseconds{esp_timer_get_time()});

    const int64_t now_us = esp_timer_get_time();
    if (now_us - m_stacks_logged_us_ >= stacks_log_interval_us) {
        m_stacks_logged_us_ = now_us;
        log_stack_use();
    }
}

//--------------------------------------------------------------------------------------------------

void EspSystemInfoService::log_stack_use() const
{
    FixedString<stacks_line_capacity> line;
    line.append("stack used (bytes):");
    for (const TaskStack& stack : m_tasks_) {
        TaskHandle_t task = xTaskGetHandle(stack.m_name);
        if (task != nullptr) {
            // the high-water mark: the fewest bytes the stack has had left
            const uint32_t used = stack.m_bytes - uxTaskGetStackHighWaterMark(task);
            line.append(" ").append(std::string_view{stack.m_name}).append(" ");
            line.append_number(used).append("/").append_number(stack.m_bytes);
        }
    }
    ESP_LOGI(log_tag, "%s", line.c_str());
}

}  // namespace avela
