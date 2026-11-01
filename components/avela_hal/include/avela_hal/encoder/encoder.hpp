// Every rotary encoder, whatever reads it (PcntEncoder): the sender of a Turn, so a receiver can tell the encoders
// apart. An empty base for identity only, no virtual functions: what an encoder does is InputConcept
// (avela_hal/input/input_concept.hpp). Not copyable: a copy would be another sender.
#pragma once

namespace avela {

class Encoder {
public:
    Encoder(const Encoder&) = delete;
    Encoder& operator=(const Encoder&) = delete;
    Encoder(Encoder&&) = delete;
    Encoder& operator=(Encoder&&) = delete;

protected:
    Encoder() = default;
    ~Encoder() = default;
};

}  // namespace avela
