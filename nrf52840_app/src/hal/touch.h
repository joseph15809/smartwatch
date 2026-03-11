#pragma once
#include <stdint.h>

namespace touch {

    struct Point {
        int16_t x = 0;
        int16_t y = 0;
    };

    enum class Gesture : uint8_t {
        NONE        = 0x00,
        SWIPE_UP    = 0x01,
        SWIPE_DOWN  = 0x02,
        SWIPE_LEFT  = 0x03,
        SWIPE_RIGHT = 0x04,
        SINGLE_TAP  = 0x05,
        DOUBLE_TAP  = 0x0B,
        LONG_PRESS  = 0x0C,
    };

    void init();
    void touch_isr();
    void poll();   // call from app::loop() alongside buttons::poll()

    // Latest touch state — valid after poll()
    bool     touched();
    Point    point();
    Point start_point();
    Gesture  gesture();

}