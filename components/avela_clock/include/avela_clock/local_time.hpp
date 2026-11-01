// The time of day as the app shows it: local time (the configured time zone) and whether the clock has been set. A
// header of its own because a skin takes it (skin API: status_tray_skin), and the skin API headers stay
// platform-free; SystemClock::now() makes it.
#pragma once

namespace avela {

struct LocalTime {
    int m_year = 0;
    int m_month = 0;  // 1..12
    int m_day = 0;
    int m_weekday = 0;  // 0 = Sunday
    int m_hour = 0;
    int m_minute = 0;
    int m_second = 0;
    bool m_synchronised = false;  // whether anything has set the clock since power-on; if not, this is January 1970
};

}  // namespace avela
