#include "rtc.h"
#include <Arduino.h>
#include <nrf.h>
#include <nrf_rtc.h>

namespace
{
    // days in month, index 0 is unused
    static const uint8_t kDaysInMonth[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    static volatile uint32_t s_epoch_sec = 0;
    static rtc::DateTime s_base; 
    static uint32_t s_base_epoch = 0;

    inline bool is_leap(uint16_t y)
    {
        return (y % 4 == 0  && y % 100 != 0) || (y % 400 == 0);
    }

    inline uint8_t days_in_month(uint8_t m, uint16_t y)
    {
        if (m == 2 && is_leap(y)) return 29;
        return kDaysInMonth[m];
    }

    // advance a DateTime by 'delta' seconds 
    rtc::DateTime advance(rtc::DateTime dt, uint32_t delta)
    {
        uint32_t s = dt.sec + delta;
        dt.sec = s % 60;
        uint32_t carry_min = s / 60;

        
        uint32_t min = dt.min + carry_min;
        dt.min = min % 60;
        uint32_t carry_hr = min / 60;

        
        uint32_t hr = dt.hour + carry_hr;
        dt.hour = hr % 24;
        uint32_t carry_day = hr / 24;

        dt.wday = (dt.wday + (uint8_t)carry_day) % 7;

        while(carry_day > 0)
        {
            uint8_t dim = days_in_month(dt.month, dt.year);
            uint32_t remaining = dim - dt.day;
            if(carry_day <= remaining)
            {
                dt.day += (uint8_t)carry_day;
                carry_day = 0;
            }
            else
            {
                carry_day -= (remaining + 1);
                dt.day =  1;
                dt.month++;
                if(dt.month > 12)
                {
                    dt.year++;
                    dt.month = 1;
                }
            }
        }
        return dt;
    }

    extern "C" void RTC2_IRQHandler(void)
    {
        if(NRF_RTC2->EVENTS_COMPARE[0])
        {
            NRF_RTC2->EVENTS_COMPARE[0] = 0;
            s_epoch_sec++;
            NRF_RTC2->CC[0] = (NRF_RTC2->COUNTER + 32768) & 0xFFFFFF;
        }
    }
}

namespace rtc
{
    void init(const DateTime& dt)
    {
        s_base = dt;
        s_base_epoch = 0;
        s_epoch_sec = 0;

        // RTC2 config
        NRF_RTC2->TASKS_STOP = 1;
        NRF_RTC2->TASKS_CLEAR = 1;
        NRF_RTC2->PRESCALER = 0; // 32768 Hz, max resolution
        NRF_RTC2->CC[0] = 32768; // 1 sec
        NRF_RTC2->INTENSET = RTC_INTENCLR_COMPARE0_Msk;
        NRF_RTC2->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;

        NVIC_SetPriority(RTC2_IRQn, 6); // low prio
        NVIC_ClearPendingIRQ(RTC2_IRQn);
        NVIC_EnableIRQ(RTC2_IRQn);

        NRF_RTC2->TASKS_START = 1;
    }

    void set(const DateTime& dt)
    {
        noInterrupts();
        s_base = dt;
        s_base_epoch = s_epoch_sec;
        interrupts();
    }

    DateTime now()
    {
        uint32_t delta;
        noInterrupts();
        delta = s_epoch_sec - s_base_epoch;
        DateTime base = s_base;
        interrupts();

        if (delta == 0) return base;
        return advance(base, delta);
    }

    uint8_t hour() { return now().hour;}
    uint8_t min()  { return now().min; }
    uint8_t sec()  { return now().sec; }
} // namespace rtc

