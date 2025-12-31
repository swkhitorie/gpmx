#include <device/rtc.h>

static bool _is_leap_year(int year)
{
    /** year is not offset */
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

static const uint8_t _days_in_month[2][12] =
{
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, /* common year */
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}  /* leap year */
};

static uint32_t _days_since_1970(uint32_t year)
{
    uint32_t y = year - 1970;
    uint32_t leaps = (y + 1) / 4 - (y + 69) / 100 + (y + 369) / 400;
    return y * 365 + leaps;
}

bool rclk_timstamp_to_tm(rclk_time_t timestamp, struct rclk_tm *tm)
{
    if (tm == NULL) return false;

    uint64_t days, rem_sec;
    uint32_t year = 1970;
    int month;
    int year_leap;
    const uint8_t *month_days;

    days = timestamp / 86400LL;
    rem_sec = timestamp % 86400LL;
    // if (rem_sec < 0) { /* before 1970 */
    //     rem_sec += 86400LL;
    //     days--;
    // }

    /* 2. 1970/1/1 Thursday */
    tm->_wday = (int)((days + 4) % 7);
    // if (tm->_wday < 0) tm->_wday += 7;

    tm->_hour = (int)(rem_sec / 3600);
    rem_sec %= 3600;
    tm->_min = (int)(rem_sec / 60);
    tm->_sec = (int)(rem_sec % 60);

    // while (days < 0 || days >= (_is_leap_year(year) ? 366 : 365)) {
    //     long long days_in_year = _is_leap_year(year) ? 366 : 365;
    //     if (days < 0) {
    //         year--;
    //         days += _is_leap_year(year) ? 366 : 365;
    //     } else {
    //         days -= days_in_year;
    //         year++;
    //     }
    // }
    while (days >= (_is_leap_year(year) ? 366ULL : 365ULL)) {
        days -= _is_leap_year(year) ? 366ULL : 365ULL;
        year++;
    }

    tm->_year = (int)(year - 1900);

    year_leap = _is_leap_year(year) ? 1 : 0;
    month_days = _days_in_month[year_leap];
    month = 0;
    while (days >= month_days[month]) {
        days -= month_days[month];
        month++;
    }
    tm->_mon = month;
    tm->_mday = (int)days + 1;

    return true;
}

bool rclk_tm_to_timstamp(const struct rclk_tm *tm, rclk_time_t *timestamp)
{
    if (tm == NULL || timestamp == NULL) return false;

    if (tm->_mon < 0 || tm->_mon > 11) return false;
    if (tm->_mday < 1 || tm->_mday > 31) return false;
    if (tm->_hour < 0 || tm->_hour > 23) return false;
    if (tm->_min < 0 || tm->_min > 59) return false;
    if (tm->_sec < 0 || tm->_sec > 60) return false;

    uint64_t seconds = 1;
    uint32_t year;
    int month, year_leap;
    const uint8_t *month_days;

    year = (uint32_t)(tm->_year + 1900);
    if (year < 1970) return false;

    seconds = (uint64_t)_days_since_1970(year) * 86400ULL;

    year_leap = _is_leap_year((int)year) ? 1 : 0;
    month_days = _days_in_month[year_leap];
    for (month = 0; month < tm->_mon; month++) {
        seconds += (uint64_t)month_days[month] * 86400ULL;
    }

    seconds += (uint64_t)(tm->_mday - 1) * 86400ULL;
    seconds += (uint64_t)tm->_hour * 3600ULL;
    seconds += (uint64_t)tm->_min * 60ULL;
    seconds += (uint64_t)tm->_sec;

    // if (seconds > UINT32_MAX) {
    //     return false;
    // }

    *timestamp = (rclk_time_t)seconds;
    return true;
}
