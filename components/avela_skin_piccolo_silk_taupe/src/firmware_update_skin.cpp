#include "avela_skin_piccolo_silk_taupe/firmware_update_skin.hpp"

#include <algorithm>
#include <array>
#include <span>

#include "avela_skin_piccolo_silk_taupe/firmware_update_layout.hpp"
#include "avela_skin_piccolo_silk_taupe/palette.hpp"
#include "avela_skin_piccolo_silk_taupe/widgets.hpp"

namespace avela::skin_piccolo_silk_taupe {
namespace {

constexpr int whole_percent = 100;

}  // namespace

//--------------------------------------------------------------------------------------------------

void FirmwareUpdateSkin::set_running_version(std::string_view version)
{
    if (keep_text(m_version_, version)) {
        render();
    }
}

//--------------------------------------------------------------------------------------------------

void FirmwareUpdateSkin::set_update_page(std::string_view by_host_name, std::string_view by_address)
{
    bool changed = keep_text(m_page_by_host_name_, by_host_name);
    changed = keep_text(m_page_by_address_, by_address) || changed;
    if (changed) {
        render();
    }
}

//--------------------------------------------------------------------------------------------------

void FirmwareUpdateSkin::set_progress(FirmwareUpdateState state, int percent, std::string_view failure)
{
    bool changed = state != m_state_ || percent != m_percent_;
    m_state_ = state;
    m_percent_ = std::clamp(percent, 0, whole_percent);
    changed = keep_text(m_failure_, failure) || changed;
    if (changed) {
        render();
    }
}

//--------------------------------------------------------------------------------------------------

void FirmwareUpdateSkin::draw()
{
    m_shown_ = true;
    render();
}

//--------------------------------------------------------------------------------------------------

void FirmwareUpdateSkin::render() const
{
    if (!m_shown_) {
        return;
    }
    const Fonts& fonts = m_surface_.m_fonts;

    LabelText running;
    running.append("Running ").append(m_version_.view());
    LabelText state;
    switch (m_state_) {
    case FirmwareUpdateState::waiting:
        state.append("Waiting for the new firmware…");
        break;
    case FirmwareUpdateState::receiving:
        state.append("Receiving… ").append_number(m_percent_).append("%");
        break;
    case FirmwareUpdateState::restarting:
        state.append("Installed. Restarting…");
        break;
    case FirmwareUpdateState::failed:
        state.append(m_failure_.view());
        break;
    }
    const bool can_leave = m_state_ == FirmwareUpdateState::waiting || m_state_ == FirmwareUpdateState::failed;

    struct Row {
        const char* m_label;
        const char* m_value;
    };
    const std::array rows{
        Row{.m_label = "Open", .m_value = m_page_by_host_name_.c_str()},
        Row{.m_label = "or", .m_value = m_page_by_address_.c_str()},
    };
    const std::size_t row_count = m_page_by_address_.view().empty() ? 1 : rows.size();

    m_surface_.m_canvas.render(layout::content, palette::background, [&](Painter& painter) {
        constexpr const auto& update = layout::firmware_update;
        draw_badge(painter, update.m_badge, "UPDATE", fonts);
        painter.text("Firmware update", update.m_headline.m_x, update.m_headline.m_y, fonts.m_headline.get(),
                     palette::text);
        painter.text(running.c_str(), update.m_status_line.m_x, update.m_status_line.m_y, fonts.m_body.get(),
                     palette::text_secondary);
        int y = update.m_rows_top;
        for (const Row& row : std::span(rows).first(row_count)) {
            painter.text(row.m_label, update.m_label_x, y, fonts.m_body.get(), palette::label);
            painter.text(row.m_value, update.m_value_x, y, fonts.m_body_strong.get(), palette::text);
            y += update.m_row_height;
        }

        const Rect& bar = update.m_progress;
        painter.fill_round_rect(bar, bar.m_height / 2, palette::surface);
        if (m_percent_ > 0) {
            const int filled = std::max(bar.m_height, bar.m_width * m_percent_ / whole_percent);
            const Rect done{.m_x = bar.m_x, .m_y = bar.m_y, .m_width = filled, .m_height = bar.m_height};
            painter.fill_round_rect(done, bar.m_height / 2, palette::accent);
            draw_gold_glint(painter, done, bar.m_height / 2);
        }
        const uint32_t state_color = m_state_ == FirmwareUpdateState::failed ? palette::accent : palette::text;
        painter.text(state.c_str(), update.m_state.m_x, update.m_state.m_y, fonts.m_caption.get(), state_color);
        if (can_leave) {
            painter.text("Back: cancel", update.m_hint.m_x, update.m_hint.m_y, fonts.m_caption.get(), palette::label,
                         lgfx::textdatum_t::top_right);
        }
    });
}

}  // namespace avela::skin_piccolo_silk_taupe
