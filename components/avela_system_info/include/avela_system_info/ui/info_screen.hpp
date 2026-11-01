// Board information (Info tile): its view and controller.
#pragma once

#include <utility>

#include "avela_network/model/network_model.hpp"
#include "avela_system_info/model/system_info_model.hpp"
#include "avela_system_info/ui/info_controller.hpp"
#include "avela_system_info/ui/info_skin_concept.hpp"
#include "avela_system_info/ui/info_view.hpp"
#include "avela_ui/screen_id.hpp"

namespace avela {

template <InfoSkinConcept PageSkin>
struct InfoScreen {
    static constexpr ScreenId id = ScreenId::info;

    InfoScreen(const SystemInfoModel& system_info, const NetworkModel& network, PageSkin skin)
        : m_view{system_info, network, std::move(skin)}
    {}

    InfoView<PageSkin> m_view;
    InfoController m_controller;
};

}  // namespace avela
