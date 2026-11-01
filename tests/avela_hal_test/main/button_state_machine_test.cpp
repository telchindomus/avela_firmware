#include "avela_hal/button/button_state_machine.hpp"

#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <variant>

#include "avela_hal/button/button.hpp"

namespace avela {
namespace {

class TestButton : public Button {};

constexpr ButtonTiming timing{.m_debounce_ms = 20, .m_long_press_ms = 600, .m_double_click_ms = 300};

// Holds the level from `from_ms` to `to_ms` (excluded) in 5 ms polls; the last event seen, if any.
std::optional<InputEvent> hold(ButtonStateMachine& machine, const Button& sender, bool pressed, uint32_t from_ms,
                               uint32_t to_ms)
{
    std::optional<InputEvent> last;
    for (uint32_t now = from_ms; now < to_ms; now += 5) {
        if (const std::optional<InputEvent> event = machine.update(pressed, now, sender)) {
            last = event;
        }
    }
    return last;
}

TEST_CASE("a click is sent by the button that was pressed")
{
    const TestButton button;
    ButtonStateMachine machine{timing};
    CHECK_FALSE(hold(machine, button, true, 0, 100).has_value());
    const InputEvent event = hold(machine, button, false, 100, 500).value_or(Turn{});  // Turn: none came
    REQUIRE(std::holds_alternative<Click>(event));
    CHECK(std::get<Click>(event).m_sender == &button);
}

TEST_CASE("double click and long press carry the sender too")
{
    const TestButton button;
    ButtonStateMachine double_machine{timing};
    hold(double_machine, button, true, 0, 50);
    hold(double_machine, button, false, 50, 100);
    hold(double_machine, button, true, 100, 150);
    const InputEvent double_click = hold(double_machine, button, false, 150, 200).value_or(Turn{});
    REQUIRE(std::holds_alternative<DoubleClick>(double_click));
    CHECK(std::get<DoubleClick>(double_click).m_sender == &button);

    ButtonStateMachine long_machine{timing};
    const InputEvent long_press = hold(long_machine, button, true, 0, 700).value_or(Turn{});
    REQUIRE(std::holds_alternative<LongPress>(long_press));
    CHECK(std::get<LongPress>(long_press).m_sender == &button);
}

}  // namespace
}  // namespace avela
