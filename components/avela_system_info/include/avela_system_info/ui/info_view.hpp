// Board information (Info tile): hands the network (NetworkModel), internal RAM, firmware and uptime
// (SystemInfoModel) to its skin.
#pragma once

#include <utility>

#include "avela_network/model/network_model.hpp"
#include "avela_system_info/model/system_info_model.hpp"
#include "avela_system_info/ui/info_skin_concept.hpp"

namespace avela {

template <InfoSkinConcept PageSkin>
class InfoView {
public:
    InfoView(const SystemInfoModel& system_info, const NetworkModel& network, PageSkin skin)
        : m_system_info_(system_info), m_network_(network), m_skin_(std::move(skin))
    {}

    void draw()
    {
        refresh();
        m_skin_.draw();
    }

    void refresh()
    {
        m_skin_.set_network(m_network_.m_status, m_network_.m_ssid.view(), m_network_.m_address.view(),
                            m_network_.m_host_name.view(), m_network_.m_signal_dbm);
        m_skin_.set_memory(m_system_info_.m_free_kilobytes, m_system_info_.m_minimum_free_kilobytes,
                           m_system_info_.m_total_kilobytes);
        m_skin_.set_firmware(m_system_info_.m_firmware_version, m_system_info_.m_uptime);
    }

private:
    const SystemInfoModel& m_system_info_;
    const NetworkModel& m_network_;
    PageSkin m_skin_;
};

}  // namespace avela
