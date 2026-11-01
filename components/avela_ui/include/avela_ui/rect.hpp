// A rectangle on the screen, in screen coordinates.
#pragma once

namespace avela {

struct Rect {
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;

    [[nodiscard]] constexpr int right() const { return m_x + m_width; }
    [[nodiscard]] constexpr int bottom() const { return m_y + m_height; }
};

}  // namespace avela
