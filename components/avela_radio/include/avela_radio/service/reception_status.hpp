// How the stream task's connection goes (ReceptionReport).
#pragma once

#include <cstdint>

namespace avela {

enum class ReceptionStatus : uint8_t {
    idle,        // asked to stop
    connecting,  // until the first chunk is queued
    receiving,
    failed,  // until the next try
};

}  // namespace avela
