#include "avela_hal/memory/heap_watch.hpp"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"

namespace avela {
namespace {

constexpr std::size_t bytes_per_kilobyte = 1024;
constexpr int64_t microseconds_per_millisecond = 1000;

std::size_t free_kilobytes()
{
    return heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / bytes_per_kilobyte;
}

}  // namespace

//--------------------------------------------------------------------------------------------------

HeapWatch::HeapWatch()
    : m_free_before_(free_kilobytes()), m_started_us_(esp_timer_get_time()),
      m_watching_lowest_(heap_caps_monitor_local_minimum_free_size_start() == ESP_OK)
{}

//--------------------------------------------------------------------------------------------------

HeapWatch::~HeapWatch()
{
    if (m_watching_lowest_) {
        heap_caps_monitor_local_minimum_free_size_stop();
    }
}

//--------------------------------------------------------------------------------------------------

void HeapWatch::log(const char* tag, const char* what) const
{
    const int64_t elapsed_ms = (esp_timer_get_time() - m_started_us_) / microseconds_per_millisecond;
    if (m_watching_lowest_) {
        ESP_LOGI(tag, "%s in %lld ms: heap %zu KB before, %zu KB lowest, %zu KB after", what, elapsed_ms,
                 m_free_before_, heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL) / bytes_per_kilobyte,
                 free_kilobytes());
    } else {
        ESP_LOGI(tag, "%s in %lld ms: heap %zu KB before, %zu KB after (lowest not measured: another watch ran)", what,
                 elapsed_ms, m_free_before_, free_kilobytes());
    }
}

}  // namespace avela
