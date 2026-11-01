// Measures the internal heap around a piece of work (a connection, a request): the free heap before, the lowest
// while it ran and the free heap after, and how long it took. What a connection really costs, logged by the one that
// opens it; RAM is the scarce thing on the board.
// ESP-IDF tracks the lowest over one window at a time: a watch started while another one runs measures no lowest.
#pragma once

#include <cstddef>
#include <cstdint>

namespace avela {

class HeapWatch {
public:
    // Starts watching.
    HeapWatch();
    HeapWatch(const HeapWatch&) = delete;
    HeapWatch& operator=(const HeapWatch&) = delete;
    HeapWatch(HeapWatch&&) = delete;
    HeapWatch& operator=(HeapWatch&&) = delete;
    ~HeapWatch();

    // One line under `tag`: "<what> in 820 ms: heap 61 KB before, 38 KB lowest, 52 KB after".
    void log(const char* tag, const char* what) const;

private:
    std::size_t m_free_before_;
    int64_t m_started_us_;
    bool m_watching_lowest_;
};

}  // namespace avela
