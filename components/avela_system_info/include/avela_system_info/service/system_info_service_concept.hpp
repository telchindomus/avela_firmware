// What a system service provides: it reports the device's state into the SystemInfoModel. Implementation:
// EspSystemInfoService (ESP-IDF); a desktop build would bring its own. The app picks one at compile time.
#pragma once

#include <concepts>

#include "avela_system_info/model/system_info_model.hpp"

namespace avela {

template <typename T>
concept SystemInfoServiceConcept = requires(T& service, SystemInfoModel& model) {
    { service.update(model) } -> std::same_as<void>;  // every frame: write the reports
};

}  // namespace avela
