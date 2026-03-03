#include "app.h"
#include "../ui/ui.h"
#include "../hal/buttons.h"
#include <Arduino.h>

namespace
{
    // event circular queue
    constexpr int QSIZE = 16;
    Event q[QSIZE];
    volatile int head = 0, tail = 0;

    // push on top of queue
    bool q_push(const Event& e)
    {
        int n = (head + 1) % QSIZE;
        if (n == tail) return false; // returns if q is full
        q[head] = e;
        head = n;
        return true;
    }

    // pop from tail
    bool q_pop(Event& out)
    {
        if (tail == head) return false; // q is empty
        out = q[tail];
        tail = (tail + 1) % QSIZE;
        return true;
    }

    uint32_t last10 = 0;
    uint32_t last1s = 0;
}

namespace app
{
    void init(){
        ui::init();
        buttons::init();
    }

    void post(const Event& e)
    {
        q_push(e);
    }

    void loop()
    {
        // periodic ticks, replays delay()
        uint32_t now = millis();
        if (now - last10 >= 10) { last10 = now; post({EventType::TICK_10MS}); }
        if (now - last1s >= 1000) { last1s = now; post({EventType::TICK_1S}); }

        // poll button, will later be interrupt
        button::poll();

        // sends events
        Event e;
        while (q_pop(e))
        {
            ui::handle(e);
        }

        // lets LVGL always run
        ui::tick();
    }
}