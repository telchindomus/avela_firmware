#include "avela_skin_piccolo/info_skin.hpp"

#include <array>

#include "avela_skin_piccolo/info_layout.hpp"
#include "avela_skin_piccolo/palette.hpp"
#include "avela_strings/fixed_string.hpp"
#include "sdkconfig.h"

namespace avela::skin_piccolo {
namespace {

constexpr const char* not_applicable = "–";
constexpr const char* not_configured = "Not configured";  // no home network saved

}  // namespace

//--------------------------------------------------------------------------------------------------

void InfoSkin::set_network(WifiStatus status, std::string_view ssid, std::string_view address,
                           std::string_view host_name, int signal_dbm)
{
    bool changed = status != m_status_ || signal_dbm != m_signal_dbm_;
    m_status_ = status;
    m_signal_dbm_ = signal_dbm;
    changed = keep_text(m_ssid_, ssid) || changed;
    changed = keep_text(m_address_, address) || changed;
    changed = keep_text(m_host_name_, host_name) || changed;
    if (changed) {
        render_details();
    }
}

//--------------------------------------------------------------------------------------------------

void InfoSkin::set_memory(std::size_t free_kilobytes, std::size_t minimum_free_kilobytes, std::size_t total_kilobytes)
{
    if (free_kilobytes == m_free_kilobytes_ && minimum_free_kilobytes == m_minimum_free_kilobytes_ &&
        total_kilobytes == m_total_kilobytes_) {
        return;
    }
    m_free_kilobytes_ = free_kilobytes;
    m_minimum_free_kilobytes_ = minimum_free_kilobytes;
    m_total_kilobytes_ = total_kilobytes;
    render_details();
}

//--------------------------------------------------------------------------------------------------

void InfoSkin::set_firmware(std::string_view version, std::chrono::seconds uptime)
{
    const auto uptime_minutes = std::chrono::duration_cast<std::chrono::minutes>(uptime);
    bool changed = uptime_minutes != m_uptime_;
    m_uptime_ = uptime_minutes;
    changed = keep_text(m_version_, version) || changed;
    if (changed) {
        render_details();
    }
}

//--------------------------------------------------------------------------------------------------

void InfoSkin::draw()
{
    m_shown_ = true;
    m_surface_.m_canvas.render(layout::content, palette::background, [&](Painter& painter) {
        draw_badge(painter, layout::info.m_badge, "INFO", m_surface_.m_fonts);
        draw_details(painter);
    });
}

//--------------------------------------------------------------------------------------------------

void InfoSkin::render_details() const
{
    if (m_shown_) {
        m_surface_.m_canvas.render(layout::info.m_details_area, palette::background,
                                   [&](Painter& painter) { draw_details(painter); });
    }
}

//--------------------------------------------------------------------------------------------------

void InfoSkin::draw_details(Painter& painter) const
{
    namespace chrono = std::chrono;
    constexpr const auto& info = layout::info;
    constexpr std::size_t percent = 100;

    const Fonts& fonts = m_surface_.m_fonts;
    const auto uptime_hours = chrono::duration_cast<chrono::hours>(m_uptime_);
    const auto uptime_minutes = m_uptime_ - uptime_hours;
    const bool connected = m_status_ == WifiStatus::connected;

    LabelText network;
    if (m_status_ == WifiStatus::not_configured) {
        network.append(not_configured);
    } else {
        network.append(m_ssid_.view()).append(connected ? "" : " · joining");
    }
    LabelText address;
    LabelText signal;
    if (connected) {
        address.append(m_address_.view()).append(" · ").append(m_host_name_.view()).append(".local");
        signal.append_number(m_signal_dbm_).append(" dBm");
    } else {
        address.append(not_applicable);
        signal.append(not_applicable);
    }
    LabelText memory;
    memory.append_number(m_free_kilobytes_)
        .append(" KB free · min ")
        .append_number(m_minimum_free_kilobytes_)
        .append(" KB");
    LabelText firmware;
    firmware.append(m_version_.view())
        .append(" · up ")
        .append_number(uptime_hours.count())
        .append(" h ")
        .append_number(uptime_minutes.count(), 2)
        .append(" min");

    struct Row {
        const char* m_label;
        const char* m_value;
    };
    const std::array rows{
        Row{.m_label = "Network", .m_value = network.c_str()},   Row{.m_label = "Address", .m_value = address.c_str()},
        Row{.m_label = "Signal", .m_value = signal.c_str()},     Row{.m_label = "Memory", .m_value = memory.c_str()},
        Row{.m_label = "Firmware", .m_value = firmware.c_str()},
    };
    int y = info.m_rows_top;
    for (const Row& row : rows) {
        painter.text(row.m_label, info.m_label_x, y, fonts.m_body.get(), palette::label);
        painter.text(row.m_value, info.m_value_x, y, fonts.m_body_strong.get(), palette::text);
        y += info.m_row_height;
    }

    // free internal RAM as a ring, in whole percent
    const std::size_t free_percent =
        m_total_kilobytes_ > 0 ? (m_free_kilobytes_ * percent + m_total_kilobytes_ / 2) / m_total_kilobytes_ : 0;
    const auto ring_x = static_cast<float>(info.m_ring_center.m_x);
    const auto ring_y = static_cast<float>(info.m_ring_center.m_y);
    painter.ring(ring_x, ring_y, info.m_ring_radius, info.m_ring_thickness, 0.0f, full_turn_degrees, palette::surface,
                 palette::background);
    painter.ring(ring_x, ring_y, info.m_ring_radius, info.m_ring_thickness, twelve_o_clock_degrees,
                 full_turn_degrees * static_cast<float>(free_percent) / static_cast<float>(percent), palette::accent,
                 palette::background);
    FixedString<number_capacity<std::size_t> + capacity_for("%")> percent_text;
    percent_text.append_number(free_percent).append("%");
    painter.text(percent_text.c_str(), info.m_ring_center.m_x, info.m_ring_center.m_y, fonts.m_figure.get(),
                 palette::text, lgfx::textdatum_t::middle_center);
    painter.text("RAM free", info.m_ring_caption.m_x, info.m_ring_caption.m_y, fonts.m_caption.get(),
                 palette::text_secondary, lgfx::textdatum_t::middle_center);

    painter.text("Hold: Wi-Fi setup · Double-click: update", info.m_hint.m_x, info.m_hint.m_y, fonts.m_caption.get(),
                 palette::text_secondary);
}

}  // namespace avela::skin_piccolo
