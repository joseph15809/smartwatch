#include "buttons.h"
#include "../app/app.h"
#include "../../include/config.h"
#include "../../include/lv_conf.h"
#include <Arduino.h>

namespace
{
    bool last = true; // last button state
    uint32_t t_down = 0; // last time button state change
    bool long_sent = false; // long button press
}

namespace buttons
{
    void init()
    {
        pinMode(PIN_BTN, INPUT_PULLDOWN);
    }

    void poll()
    {
        bool cur = digitalRead(PIN_BTN); // HIGH == not pressed
        uint32_t now = millis();
        if (last && !cur) // button pressed
        {
            t_down = now;
            long_sent = false;
        }

        // check if long button press
        if (!last && !cur && !long_sent && (now - t_down) > 600)
        {
            app::post(Event(EventType::BTN_LONG));
            long_sent = true;
        }

        if (!last && cur)
        {
            if (!long_sent && (now - t_down) > 30) // makes sure it was a short button press
            {
                app::post(Event(EventType::BTN_SHORT));
            }
        }
        last = cur;
    }
}