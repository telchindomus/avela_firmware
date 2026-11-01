// A service's status, from the service on the main loop to any task (the setup page's, the console's): a latest-value
// queue, one item in static storage, created with the object; a newer status overwrites it, and readers read it
// without taking it. The status crosses tasks as a copy, so it must be trivially copyable.
#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace avela {

template <typename StatusType>
class StatusQueue {
public:
    static_assert(std::is_trivially_copyable_v<StatusType>, "the status crosses tasks as a copy");

    StatusQueue() : m_queue_(xQueueCreateStatic(1, sizeof(StatusType), m_item_.data(), &m_storage_))
    {
        xQueueOverwrite(m_queue_, &m_published_);
    }
    StatusQueue(const StatusQueue&) = delete;
    StatusQueue& operator=(const StatusQueue&) = delete;
    StatusQueue(StatusQueue&&) = delete;
    StatusQueue& operator=(StatusQueue&&) = delete;
    ~StatusQueue() = default;

    // The service, on the main loop: the status now; passed on only when it changed.
    void publish(const StatusType& status)
    {
        if (status != m_published_) {
            m_published_ = status;
            xQueueOverwrite(m_queue_, &m_published_);
        }
    }

    // Any task: the status as last published.
    [[nodiscard]] StatusType read() const
    {
        StatusType status{};
        xQueuePeek(m_queue_, &status, 0);
        return status;
    }

private:
    StatusType m_published_{};
    StaticQueue_t m_storage_{};
    std::array<uint8_t, sizeof(StatusType)> m_item_{};
    QueueHandle_t m_queue_;
};

}  // namespace avela
