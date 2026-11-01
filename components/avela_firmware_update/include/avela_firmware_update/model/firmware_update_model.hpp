// The firmware update as the update screen sees it, written by the firmware update service: where the update is, how
// much of the image has arrived, and why it failed. Whether uploads are accepted is a request
// (AcceptUploadsRequest).
#pragma once

#include <cstddef>

#include "avela_firmware_update/model/firmware_update_state.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela {

struct FirmwareUpdateModel {
    static constexpr std::size_t failure_capacity = 64;

    FirmwareUpdateState m_state = FirmwareUpdateState::waiting;
    std::size_t m_received_bytes = 0;
    std::size_t m_total_bytes = 0;  // the upload's size (0 until one starts)
    FixedString<failure_capacity> m_failure;
    const char* m_running_version = "";  // the firmware that runs now

    [[nodiscard]] int percent_received() const
    {
        constexpr std::size_t whole = 100;
        return m_total_bytes == 0 ? 0 : static_cast<int>(m_received_bytes * whole / m_total_bytes);
    }
};

}  // namespace avela
