// The firmware update screen (Info: Menu): hands where the update page is and how the upload goes to its skin.
#pragma once

#include <string_view>
#include <utility>

#include "avela_firmware_update/model/firmware_update_model.hpp"
#include "avela_firmware_update/ui/firmware_update_skin_concept.hpp"
#include "avela_network/model/network_model.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela {

template <FirmwareUpdateSkinConcept PageSkin>
class FirmwareUpdateView {
public:
    FirmwareUpdateView(const FirmwareUpdateModel& model, const NetworkModel& network, PageSkin skin)
        : m_model_(model), m_network_(network), m_skin_(std::move(skin))
    {}

    void draw()
    {
        refresh();
        m_skin_.draw();
    }

    void refresh()
    {
        m_skin_.set_running_version(m_model_.m_running_version);
        m_address_page_.clear();
        if (m_network_.m_status == WifiStatus::connected && !m_network_.m_address.view().empty()) {
            m_address_page_.append("http://").append(m_network_.m_address.view()).append(page_path);
        }
        m_host_name_page_.clear()
            .append("http://")
            .append(m_network_.m_host_name.view())
            .append(".local")
            .append(page_path);
        m_skin_.set_update_page(m_host_name_page_.view(), m_address_page_.view());
        m_skin_.set_progress(m_model_.m_state, m_model_.percent_received(), m_model_.m_failure.view());
    }

private:
    static constexpr std::string_view page_path = "/update";

    const FirmwareUpdateModel& m_model_;
    const NetworkModel& m_network_;
    PageSkin m_skin_;
    // the page by host name and by IP address, made in refresh() (the skin gets views of them)
    FixedString<capacity_for("http://") + HostName::capacity + capacity_for(".local/update")> m_host_name_page_;
    FixedString<capacity_for("http://") + NetworkModel::address_capacity + capacity_for("/update")> m_address_page_;
};

}  // namespace avela
