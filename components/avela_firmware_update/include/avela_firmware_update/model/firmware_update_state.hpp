// Where a firmware update is: what the update screen shows.
#pragma once

#include <cstdint>

namespace avela {

enum class FirmwareUpdateState : uint8_t {
    waiting,     // for an upload (the update screen is open, so one is accepted)
    receiving,   // writing the uploaded image into the other app slot
    restarting,  // the image is complete and checked; the radio restarts into it
    failed,      // the upload broke off or the image was refused; another upload may follow
};

}  // namespace avela
