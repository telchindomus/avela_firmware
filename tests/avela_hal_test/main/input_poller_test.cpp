#include "avela_hal/input/input_poller.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <optional>
#include <variant>

#include "avela_hal/button/button.hpp"
#include "avela_hal/encoder/encoder.hpp"

namespace avela {
namespace {

// Inputs that report what the test set for the next poll.
class FakeButton : public Button {
public:
    std::optional<InputEvent> poll(uint32_t /*now_ms*/)
    {
        std::optional<InputEvent> event;
        if (m_click_next) {
            event = Click{.m_sender = this};
        }
        m_click_next = false;
        return event;
    }

    bool m_click_next = false;
};

class FakeEncoder : public Encoder {
public:
    std::optional<InputEvent> poll(uint32_t /*now_ms*/)
    {
        std::optional<InputEvent> event;
        if (m_detents_next != 0) {
            event = Turn{.m_sender = this, .m_detents = m_detents_next};
        }
        m_detents_next = 0;
        return event;
    }

    int m_detents_next = 0;
};

static_assert(InputConcept<FakeButton> && InputConcept<FakeEncoder>);

TEST_CASE("a poll returns nothing when no input did anything")
{
    FakeEncoder knob;
    FakeButton button;
    InputPoller<FakeEncoder, FakeButton> poller{knob, button};
    CHECK(poller.poll(0).empty());
}

TEST_CASE("a poll returns every input's event, in the order of the inputs, with their senders")
{
    FakeEncoder knob;
    FakeButton select;
    FakeButton back;
    InputPoller<FakeEncoder, FakeButton, FakeButton> poller{knob, select, back};

    knob.m_detents_next = 2;
    back.m_click_next = true;
    const auto events = poller.poll(5);
    REQUIRE(events.size() == 2);
    REQUIRE(std::holds_alternative<Turn>(events[0]));
    CHECK(std::get<Turn>(events[0]).m_sender == &knob);
    CHECK(std::get<Turn>(events[0]).m_detents == 2);
    REQUIRE(std::holds_alternative<Click>(events[1]));
    CHECK(std::get<Click>(events[1]).m_sender == &back);

    CHECK(poller.poll(10).empty());  // reported once

    knob.m_detents_next = -1;
    select.m_click_next = true;
    back.m_click_next = true;
    CHECK(poller.poll(15).size() == 3);  // one slot per input
}

}  // namespace
}  // namespace avela
