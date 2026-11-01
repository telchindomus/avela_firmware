// One or more steps down (volume, the selection); what a step is, is the shown screen's. Made by the application from
// the knob turned anticlockwise: one detent is one step.
#pragma once

namespace avela {

struct DecrementCommand {
    int m_steps = 1;  // > 0
};

}  // namespace avela
