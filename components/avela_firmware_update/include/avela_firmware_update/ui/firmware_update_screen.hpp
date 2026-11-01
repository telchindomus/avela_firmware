// The firmware update screen (Info's child): its view and controller.
#pragma once

#include <utility>

#include "avela_firmware_update/model/firmware_update_model.hpp"
#include "avela_firmware_update/ui/firmware_update_controller.hpp"
#include "avela_firmware_update/ui/firmware_update_skin_concept.hpp"
#include "avela_firmware_update/ui/firmware_update_view.hpp"
#include "avela_network/model/network_model.hpp"
#include "avela_ui/screen_id.hpp"

namespace avela {

template <FirmwareUpdateSkinConcept PageSkin>
struct FirmwareUpdateScreen {
    static constexpr ScreenId id = ScreenId::firmware_update;

    FirmwareUpdateScreen(const FirmwareUpdateModel& model, const NetworkModel& network, PageSkin skin)
        : m_view{model, network, std::move(skin)}, m_controller{model}
    {}

    FirmwareUpdateView<PageSkin> m_view;
    FirmwareUpdateController m_controller;
};

}  // namespace avela
