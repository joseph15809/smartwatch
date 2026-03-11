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
    TOUCH_SWIPE_UP,
    TOUCH_SWIPE_DOWN,
    TOUCH_SWIPE_LEFT,
    TOUCH_SWIPE_RIGHT,
    TOUCH_LONG,
    BLE_NOTIF,
    BLE_MEDIA,
    MEDIA_PLAYPAUSE,
    MEDIA_PREV,
    MEDIA_NEXT,
    // BLE pushes this when track/state changes
    // Event.a = elapsed seconds, Event.b = duration seconds
    // title/artist come via a separate MusicState update (BLE callback)
    BLE_MEDIA_STATE,
};

struct Event
{
    EventType type;
    int32_t a = 0;
    int32_t b= 0;

    // default constructor
    constexpr Event() : type(EventType::TICK_10MS), a(0), b(0) {}

    // main constructor
    constexpr Event(EventType t, int32_t a_=0, int32_t b_=0)
    : type(t), a(a_), b(b_) {}
};
