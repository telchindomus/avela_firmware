// Files embedded by this component's CMakeLists.txt (EMBED_FILES): the piccolo skin's assets for this build's screen
// (assets/generated/<screen>/). The linker names them after the file name and marks them with start/end
// symbols; avela::assets::<name>() returns each one as a span of what the file holds (bytes, or ARGB8888 pixels for
// the logo).
#pragma once

#include <cstdint>
#include <span>

#include "avela_logo_size.hpp"
#include "lgfx/v1/misc/colortype.hpp"

// The linker symbols can only be declared as C arrays and turned into pointers; that happens here and nowhere else.
// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
// A linker symbol has no type; it is declared as an array of what the file holds.
#define AVELA_EMBEDDED_FILE(name, element)                                  \
    extern "C" const element name##_start[] asm("_binary_" #name "_start"); \
    extern "C" const element name##_end[] asm("_binary_" #name "_end");     \
    namespace avela::assets {                                               \
    inline std::span<const element> name()                                  \
    {                                                                       \
        return {name##_start, name##_end};                                  \
    }                                                                       \
    }

AVELA_EMBEDDED_FILE(font_display_vlw, uint8_t)
AVELA_EMBEDDED_FILE(font_headline_vlw, uint8_t)
AVELA_EMBEDDED_FILE(font_title_vlw, uint8_t)
AVELA_EMBEDDED_FILE(font_heading_vlw, uint8_t)
AVELA_EMBEDDED_FILE(font_figure_vlw, uint8_t)
AVELA_EMBEDDED_FILE(font_subtitle_vlw, uint8_t)
AVELA_EMBEDDED_FILE(font_list_item_vlw, uint8_t)
AVELA_EMBEDDED_FILE(font_body_vlw, uint8_t)
AVELA_EMBEDDED_FILE(font_body_strong_vlw, uint8_t)
AVELA_EMBEDDED_FILE(font_caption_vlw, uint8_t)
AVELA_EMBEDDED_FILE(font_badge_vlw, uint8_t)
AVELA_EMBEDDED_FILE(avela_logo_argb, lgfx::argb8888_t)  // raw ARGB8888, logo_width x logo_height

#undef AVELA_EMBEDDED_FILE
// NOLINTEND(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
