// Takes the stream titles out of an Icecast/SHOUTcast stream. Asked with "Icy-MetaData: 1", the station sends a
// metadata block after every icy-metaint bytes of audio (its answer's header says how many): one length byte (times 16
// bytes), then text like "StreamTitle='Artist - Title';". strip() removes those blocks from the received bytes in
// place and keeps the audio; the blocks may be split across any number of reads. Platform-free.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include "avela_strings/fixed_string.hpp"

namespace avela {

class IcyMetadataReader {
public:
    static constexpr std::size_t title_capacity = 128;
    // Only the start of a block is kept: StreamTitle comes first, and a block may be up to 4080 bytes long.
    static constexpr std::size_t metadata_capacity = 512;

    // audio_interval: the answer's icy-metaint; 0 = the station sends no metadata (every byte is audio).
    explicit IcyMetadataReader(std::size_t audio_interval)
        : m_audio_interval_(audio_interval), m_audio_left_(audio_interval)
    {}

    // Removes the metadata from bytes, in place; returns the audio part (the front of bytes).
    std::span<uint8_t> strip(std::span<uint8_t> bytes);

    // The latest StreamTitle (empty until the first one; a longer one is cut at a whole UTF-8 character); the revision
    // is raised when it changes.
    [[nodiscard]] std::string_view title() const { return m_title_.view(); }
    [[nodiscard]] uint32_t title_revision() const { return m_title_revision_; }

private:
    static constexpr std::size_t length_unit = 16;  // the length byte counts 16-byte units

    // Takes what the finished block says.
    void read_block();

    std::size_t m_audio_interval_;
    std::size_t m_audio_left_;         // audio bytes until the next length byte
    std::size_t m_metadata_left_ = 0;  // metadata bytes still to come in the current block
    std::array<char, metadata_capacity> m_metadata_{};
    std::size_t m_metadata_length_ = 0;
    FixedString<title_capacity> m_title_;
    uint32_t m_title_revision_ = 0;
};

}  // namespace avela
