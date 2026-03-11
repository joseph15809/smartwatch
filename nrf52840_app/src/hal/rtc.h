#pragma once
#include <stdint.h>

namespace rtc
{
    struct DateTime
    {
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t min;
        uint8_t sec;
        uint8_t wday; // day of the week
    };

    void init(const DateTime& now);

    // set/update current time
    void set(const DateTime& dt);

    // returns the current time
    DateTime now();

    // accessors
    uint8_t hour();
    uint8_t min();
    uint8_t sec();
}