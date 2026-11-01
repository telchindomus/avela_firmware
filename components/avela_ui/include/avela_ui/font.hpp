// A VLW font embedded in flash. LovyanGFX reads glyph bitmaps straight from flash while drawing;
// loading allocates its small per-glyph tables (about 9 bytes per glyph) once at startup.
#pragma once

#include <cstdint>
#include <cstdio>
#include <span>

#include <LovyanGFX.hpp>  // after <cstdio>: its file wrapper calls fopen() once anything has declared FILE

namespace avela {

class Font {
public:
    void load(std::span<const uint8_t> file)
    {
        m_data_.set(file.data(), static_cast<uint32_t>(file.size()));
        m_font_.loadFont(&m_data_);
    }
    [[nodiscard]] const lgfx::IFont& get() const { return m_font_; }

private:
    lgfx::PointerWrapper m_data_;
    lgfx::VLWfont m_font_;
};

}  // namespace avela
