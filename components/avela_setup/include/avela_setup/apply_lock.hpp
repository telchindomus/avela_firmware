// One section's apply() at a time, whether from the setup page or the USB console (two tasks): a section checks the
// saved settings and then saves, and that is one step only if no other apply() runs in between (the stations: one
// must be left). Held by the server and the console around every apply().
#pragma once

namespace avela {

class ApplyLock {
public:
    ApplyLock();
    ApplyLock(const ApplyLock&) = delete;
    ApplyLock& operator=(const ApplyLock&) = delete;
    ApplyLock(ApplyLock&&) = delete;
    ApplyLock& operator=(ApplyLock&&) = delete;
    ~ApplyLock();
};

}  // namespace avela
