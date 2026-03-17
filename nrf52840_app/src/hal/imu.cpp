#include "imu.h"
#include <LSM6DS3.h>
#include <Wire.h>

namespace
{
    LSM6DS3  s_imu(I2C_MODE, 0x6A);
    bool     s_ok     = false;
    uint32_t s_steps  = 0;
    uint16_t s_dist_m = 0;

    // Stride length based on user height 5'8" (172.72 cm): height × 0.415 ≈ 717 mm
    constexpr uint32_t STRIDE_MM = 717;
}

namespace imu
{
    void init()
    {
        s_ok = (s_imu.begin() == 0);
        if (!s_ok) return;

        // Enable pedometer: set PEDO_EN (bit 2) in CTRL10_C (0x19)
        // LSM6DS3 datasheet table 58 — bit 2 = PEDO_EN
        uint8_t ctrl10 = 0;
        s_imu.readRegister(&ctrl10, 0x19);
        s_imu.writeRegister(0x19, ctrl10 | 0x04);
    }

    void poll()
    {
        if (!s_ok) return;
        // Read 16-bit step counter from STEP_COUNTER_L/H (0x4B–0x4C)
        uint8_t lo = 0, hi = 0;
        s_imu.readRegister(&lo, 0x4B);
        s_imu.readRegister(&hi, 0x4C);
        s_steps  = (uint32_t)lo | ((uint32_t)hi << 8);
        s_dist_m = (uint16_t)((s_steps * STRIDE_MM) / 1000u);
    }

    uint32_t steps()      { return s_steps;  }
    uint16_t distance_m() { return s_dist_m; }
}
