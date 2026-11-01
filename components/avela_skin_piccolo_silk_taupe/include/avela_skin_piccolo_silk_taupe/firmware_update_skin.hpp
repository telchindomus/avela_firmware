// The Silk Taupe skin's firmware update screen: UPDATE badge, headline, the running version, where to upload the new
// firmware (by host name, and by IP address while connected), a gold progress bar with its glint and the update's
// state. Redraws whole when anything changes (the progress at most once per percent). Fulfils FirmwareUpdateSkin
// (avela_firmware_update).
#pragma once

#include <string_view>

#include "avela_firmware_update/model/firmware_update_model.hpp"
#include "avela_firmware_update/ui/firmware_update_skin_concept.hpp"
#include "avela_network/model/network_address.hpp"
#include "avela_skin_piccolo_silk_taupe/surface.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela::skin_piccolo_silk_taupe {

class FirmwareUpdateSkin {
public:
    explicit FirmwareUpdateSkin(const Surface& surface) : m_surface_(surface) {}

    void set_running_version(std::string_view version);
    void set_update_page(std::string_view by_host_name, std::string_view by_address);
    void set_progress(FirmwareUpdateState state, int percent, std::string_view failure);
    void draw();

private:
    static constexpr std::size_t version_capacity = 32;  // esp_app_desc_t's version field
    static constexpr std::size_t page_capacity =
        capacity_for("http://") + network_address_capacity + capacity_for("/update");

    void render() const;

    Surface m_surface_;
    bool m_shown_ = false;
    FixedString<version_capacity> m_version_;
    FixedString<page_capacity> m_page_by_host_name_;
    FixedString<page_capacity> m_page_by_address_;
    FirmwareUpdateState m_state_ = FirmwareUpdateState::waiting;
    int m_percent_ = 0;
    FixedString<FirmwareUpdateModel::failure_capacity> m_failure_;
};

}  // namespace avela::skin_piccolo_silk_taupe

namespace avela {
static_assert(FirmwareUpdateSkinConcept<skin_piccolo_silk_taupe::FirmwareUpdateSkin>);
}  // namespace avela
