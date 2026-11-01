#include "avela_network/model/host_name.hpp"

#include <algorithm>

#include "avela_settings/settings.hpp"
#include "sdkconfig.h"

namespace avela {

//--------------------------------------------------------------------------------------------------

HostName saved_host_name()
{
    HostName host_name{};
    if (!Settings::load(host_name) || !valid_host_name(host_name.m_name.view())) {
        host_name = HostName{};
        host_name.m_name.append(CONFIG_AVELA_HOST_NAME);
    }
    return host_name;
}

//--------------------------------------------------------------------------------------------------

bool valid_host_name(std::string_view name)
{
    if (name.empty() || name.size() > HostName::capacity - 1 || name.front() == '-' || name.back() == '-') {
        return false;
    }
    return std::ranges::all_of(name, [](char character) {
        return (character >= 'a' && character <= 'z') || (character >= '0' && character <= '9') || character == '-';
    });
}

}  // namespace avela
