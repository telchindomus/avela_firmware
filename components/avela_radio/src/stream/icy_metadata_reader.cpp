#include "avela_radio/stream/icy_metadata_reader.hpp"

#include <algorithm>

namespace avela {
namespace {

// The longest front of text that fits max_length bytes without cutting a UTF-8 character apart.
std::string_view utf8_prefix(std::string_view text, std::size_t max_length)
{
    constexpr unsigned continuation_mask = 0xC0;
    constexpr unsigned continuation_bits = 0x80;  // 10xxxxxx: inside a character
    if (text.size() <= max_length) {
        return text;
    }
    std::size_t end = max_length;
    while (end > 0 && (static_cast<unsigned char>(text[end]) & continuation_mask) == continuation_bits) {
        --end;
    }
    return text.substr(0, end);
}

}  // namespace

//--------------------------------------------------------------------------------------------------

// Three kinds of bytes take turns: audio_interval audio bytes, a length byte, that many units of metadata (often none).
std::span<uint8_t> IcyMetadataReader::strip(std::span<uint8_t> bytes)
{
    if (m_audio_interval_ == 0) {
        return bytes;
    }
    std::size_t audio_end = 0;  // the audio kept so far, moved to the front
    std::size_t position = 0;
    while (position < bytes.size()) {
        const std::size_t left = bytes.size() - position;
        if (m_audio_left_ > 0) {
            const std::size_t count = std::min(m_audio_left_, left);
            std::copy_n(bytes.begin() + static_cast<std::ptrdiff_t>(position), count,
                        bytes.begin() + static_cast<std::ptrdiff_t>(audio_end));
            audio_end += count;
            position += count;
            m_audio_left_ -= count;
        } else if (m_metadata_left_ > 0) {
            const std::size_t count = std::min(m_metadata_left_, left);
            const std::size_t kept = std::min(count, m_metadata_.size() - m_metadata_length_);
            std::copy_n(bytes.begin() + static_cast<std::ptrdiff_t>(position), kept,
                        m_metadata_.begin() + static_cast<std::ptrdiff_t>(m_metadata_length_));
            m_metadata_length_ += kept;
            position += count;
            m_metadata_left_ -= count;
            if (m_metadata_left_ == 0) {
                read_block();
                m_audio_left_ = m_audio_interval_;
            }
        } else {
            // the length byte
            m_metadata_left_ = bytes[position] * length_unit;
            m_metadata_length_ = 0;
            ++position;
            if (m_metadata_left_ == 0) {
                m_audio_left_ = m_audio_interval_;
            }
        }
    }
    return bytes.first(audio_end);
}

//--------------------------------------------------------------------------------------------------

// The block is text padded with zero bytes: key='value'; pairs. A title may itself hold "'" characters, so it ends at
// the first "';" after it.
void IcyMetadataReader::read_block()
{
    constexpr std::string_view key = "StreamTitle='";
    constexpr std::string_view title_end = "';";
    const std::string_view block{m_metadata_.data(), m_metadata_length_};
    const std::string_view text = block.substr(0, block.find('\0'));
    const std::size_t start = text.find(key);
    if (start == std::string_view::npos) {
        return;
    }
    std::string_view title = text.substr(start + key.size());
    const std::size_t end = title.find(title_end);
    title = utf8_prefix(title.substr(0, end != std::string_view::npos ? end : title.rfind('\'')),
                        decltype(m_title_)::max_length());
    if (title != m_title_.view()) {
        m_title_.clear().append(title);
        ++m_title_revision_;
    }
}

}  // namespace avela
