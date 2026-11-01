// The stream task to the player task (Mp3Player): MP3 bytes from one session.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace avela {

struct EncodedAudioChunk {
    static constexpr std::size_t capacity = 1024;
    uint32_t m_session = 0;
    std::size_t m_size = 0;
    std::array<uint8_t, capacity> m_bytes{};
};

}  // namespace avela
