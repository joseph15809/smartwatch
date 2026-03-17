#pragma once
#include <stdint.h>

namespace imu
{
    void     init();
    void     poll();         // call once per second; reads pedometer hardware
    uint32_t steps();        // total steps since boot
    uint16_t distance_m();   // estimated distance in meters (steps × stride)
}
