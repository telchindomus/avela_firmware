// Encoder counts -> whole detents: the counts of a part detent are kept for the next call, so slow turns add up.
// Platform-free; the encoder that reads the counter (PcntEncoder) feeds it the counts since its last read.
#pragma once

namespace avela {

class DetentCounter {
public:
    explicit constexpr DetentCounter(int counts_per_detent) : m_counts_per_detent_(counts_per_detent) {}

    // Adds counts (> 0 clockwise); returns the whole detents they complete.
    int add(int counts)
    {
        m_pending_counts_ += counts;
        const int detents = m_pending_counts_ / m_counts_per_detent_;
        m_pending_counts_ -= detents * m_counts_per_detent_;
        return detents;
    }

private:
    int m_counts_per_detent_;
    int m_pending_counts_ = 0;
};

}  // namespace avela
