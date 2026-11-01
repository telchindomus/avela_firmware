// A service's requests, from any task (the main loop's controllers, the setup page's, the console's) to the service,
// which takes them on the main loop, in order: a FreeRTOS queue of `Capacity` items in static storage, created with
// the object. A full queue refuses a request (the sender says "busy, try again"). The requests cross tasks as copies,
// so they must be trivially copyable.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace avela {

// Enough for a burst of knob detents within one frame (the controllers send one request per input event).
inline constexpr std::size_t request_queue_capacity = 8;

template <typename RequestType, std::size_t Capacity = request_queue_capacity>
class RequestQueue {
public:
    static_assert(std::is_trivially_copyable_v<RequestType>, "requests cross tasks as copies");

    RequestQueue() : m_queue_(xQueueCreateStatic(Capacity, sizeof(RequestType), m_items_.data(), &m_storage_)) {}
    RequestQueue(const RequestQueue&) = delete;
    RequestQueue& operator=(const RequestQueue&) = delete;
    RequestQueue(RequestQueue&&) = delete;
    RequestQueue& operator=(RequestQueue&&) = delete;
    ~RequestQueue() = default;

    // Any task: false if the queue is full (try again).
    bool send(const RequestType& request) { return xQueueSend(m_queue_, &request, 0) == pdTRUE; }

    // The service, on the main loop: the oldest request waiting, if any.
    std::optional<RequestType> take()
    {
        RequestType request;
        if (xQueueReceive(m_queue_, &request, 0) != pdTRUE) {
            return std::nullopt;
        }
        return request;
    }

private:
    StaticQueue_t m_storage_{};
    std::array<uint8_t, sizeof(RequestType) * Capacity> m_items_{};
    QueueHandle_t m_queue_;
};

}  // namespace avela
