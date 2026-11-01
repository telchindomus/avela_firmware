// A request to the network service: open the setup network (the radio's own Wi-Fi, for the setup page) or close it.
#pragma once

namespace avela {

struct SetupNetworkRequest {
    bool m_open = false;
};

}  // namespace avela
