// Polls a fixed set of inputs (references; the owner, a board, keeps the inputs) one after another, in the order
// given, and returns the events of this poll. The set is known at compile time, so the "loop" is a fold over a tuple:
// no virtual calls, no heap; at most one event per input per poll, so the buffer has one slot per input.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <tuple>

#include "avela_hal/input/events/input_event.hpp"
#include "avela_hal/input/input_concept.hpp"

namespace avela {

template <InputConcept... Inputs>
class InputPoller {
public:
    explicit InputPoller(Inputs&... inputs) : m_inputs_{inputs...} {}

    // The events since the last poll, in the order of the inputs; valid until the next poll.
    std::span<const InputEvent> poll(uint32_t now_ms)
    {
        m_count_ = 0;
        std::apply([&](auto&... input) { (add_event(input.poll(now_ms)), ...); }, m_inputs_);
        return std::span{m_events_}.first(m_count_);
    }

private:
    void add_event(const std::optional<InputEvent>& event)
    {
        if (event) {
            m_events_.at(m_count_) = *event;
            ++m_count_;
        }
    }

    std::tuple<Inputs&...> m_inputs_;
    std::array<InputEvent, sizeof...(Inputs)> m_events_{};
    std::size_t m_count_ = 0;
};

}  // namespace avela
