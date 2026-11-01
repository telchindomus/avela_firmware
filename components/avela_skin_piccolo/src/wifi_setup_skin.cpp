#include "avela_skin_piccolo/wifi_setup_skin.hpp"

#include <array>
#include <cstdint>

#include "avela_skin_piccolo/palette.hpp"
#include "avela_skin_piccolo/widgets.hpp"
#include "avela_skin_piccolo/wifi_setup_layout.hpp"

namespace avela::skin_piccolo {
namespace {

constexpr uint8_t first_qr_version = 1;  // LovyanGFX picks the smallest version the text fits

}  // namespace

//--------------------------------------------------------------------------------------------------

void WifiSetupSkin::set_saved_network(WifiStatus status, std::string_view ssid)
{
    bool changed = status != m_saved_status_;
    m_saved_status_ = status;
    changed = keep_text(m_saved_ssid_, ssid) || changed;
    if (changed) {
        render();
    }
}

//--------------------------------------------------------------------------------------------------

void WifiSetupSkin::set_setup_network(std::string_view name, std::string_view password, std::string_view join_code)
{
    bool changed = keep_text(m_setup_name_, name);
    changed = keep_text(m_setup_password_, password) || changed;
    changed = keep_text(m_join_code_, join_code) || changed;
    if (changed) {
        render();
    }
}

//--------------------------------------------------------------------------------------------------

void WifiSetupSkin::set_setup_page(bool active, std::string_view address)
{
    bool changed = active != m_setup_active_;
    m_setup_active_ = active;
    changed = keep_text(m_page_address_, address) || changed;
    if (changed) {
        render();
    }
}

//--------------------------------------------------------------------------------------------------

void WifiSetupSkin::draw()
{
    m_shown_ = true;
    render();
}

//--------------------------------------------------------------------------------------------------

void WifiSetupSkin::render() const
{
    if (!m_shown_) {
        return;
    }
    const Fonts& fonts = m_surface_.m_fonts;

    LabelText status;
    if (m_saved_status_ == WifiStatus::not_configured) {
        status.append("No network saved yet");
    } else {
        status.append("Saved: ").append(m_saved_ssid_.view());
    }
    LabelText hint;
    hint.append("Then open http://").append(m_page_address_.view());

    struct Row {
        const char* m_label;
        const char* m_value;
    };
    const std::array rows{
        Row{.m_label = "Join", .m_value = m_setup_name_.c_str()},
        Row{.m_label = "Password", .m_value = m_setup_password_.c_str()},
    };

    m_surface_.m_canvas.render(layout::content, palette::background, [&](Painter& painter) {
        constexpr const auto& setup = layout::wifi_setup;
        draw_badge(painter, setup.m_badge, "SETUP", fonts);
        const char* headline = m_setup_active_ ? "Set up Wi-Fi" : "Starting setup…";
        painter.text(headline, setup.m_headline.m_x, setup.m_headline.m_y, fonts.m_headline.get(), palette::text);
        painter.text(status.c_str(), setup.m_status_line.m_x, setup.m_status_line.m_y, fonts.m_body.get(),
                     palette::text_secondary);
        if (!m_setup_active_) {
            // nothing to join yet: no name, password or QR code until the setup network is up
            painter.text("Please wait…", setup.m_label_x, setup.m_rows_top, fonts.m_body.get(), palette::text);
            return;
        }
        int y = setup.m_rows_top;
        for (const Row& row : rows) {
            painter.text(row.m_label, setup.m_label_x, y, fonts.m_body.get(), palette::label);
            painter.text(row.m_value, setup.m_value_x, y, fonts.m_body_strong.get(), palette::text);
            y += setup.m_row_height;
        }
        painter.text(hint.c_str(), setup.m_hint.m_x, setup.m_hint.m_y, fonts.m_caption.get(), palette::text_secondary);
        painter.qr_code(m_join_code_.c_str(), setup.m_qr_code, first_qr_version, palette::qr_dark, palette::qr_light,
                        setup.m_qr_corner_radius);
        painter.text("Scan to join", setup.m_qr_caption.m_x, setup.m_qr_caption.m_y, fonts.m_caption.get(),
                     palette::text_secondary, lgfx::textdatum_t::middle_center);
    });
}

}  // namespace avela::skin_piccolo
