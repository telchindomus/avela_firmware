// One or more steps up (volume, the selection); what a step is, is the shown screen's. Made by the application from
// the knob turned clockwise: one detent is one step.
#pragma once

namespace avela {

struct IncrementCommand {
    int m_steps = 1;  // > 0
};

}  // namespace avela
