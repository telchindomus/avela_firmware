// A request to the firmware update service: accept uploads on the update page, or refuse them again. Uploads are
// accepted only while the firmware update screen is open, so updating needs someone at the radio as well as the page
// password.
#pragma once

namespace avela {

struct AcceptUploadsRequest {
    bool m_accept = false;
};

}  // namespace avela
