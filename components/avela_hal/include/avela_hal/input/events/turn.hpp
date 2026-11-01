// A turn of an encoder in whole detents (PcntEncoder). What it does is up to the receiver.
#pragma once

namespace avela {

class Encoder;

struct Turn {
    const Encoder* m_sender = nullptr;
    int m_detents = 0;  // > 0 clockwise; one detent = one click of the knob
};

}  // namespace avela
