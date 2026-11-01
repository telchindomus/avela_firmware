// What a controller does about a command: open another screen (one of its children) or close its own (back to the
// screen that opened it), make a request (avela_messaging: the Ui sends it to the service it is for), both, or
// neither (a change to its page model only, or nothing at all).
#pragma once

#include <optional>

#include "avela_messaging/messages/request.hpp"
#include "avela_ui/screen_id.hpp"

namespace avela {

struct Response {
    std::optional<ScreenId> m_show = std::nullopt;  // open this screen
    bool m_close = false;                           // close the shown screen
    std::optional<Request> m_request = std::nullopt;
};

}  // namespace avela
