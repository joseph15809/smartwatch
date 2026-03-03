#pragma once
#include <stdint.h>

enum class EventType : uint8_t
{
    TICK_10MS,
    TICK_1S,
    BTN_SHORT,
    BTN_LONG,
    // when touch screen arrives
    TOUCH_TAP,
    TOUCH_SWIPE,
    BLE_NOTIF,
    BLE_MEDIA,
};

struct Event
{
    EventType type;
    int32_t a = 0;
    int32_t b= 0;
};
