#include "app.h"
#include "../ui/ui.h"
#include "../hal/buttons.h"
#include "../hal/touch.h"
#include "../hal/ble.h"
#include "../hal/imu.h"
#include <Arduino.h>

namespace
{
    // event circular queue
    constexpr int QSIZE = 16;
    Event q[QSIZE];
    volatile uint8_t head = 0, tail = 0;

    bool q_push(const Event& e)
    {
        uint8_t n = (head + 1) % QSIZE;
        if (n == tail) return false; // returns if q is full
        q[head] = e;
        __DMB(); // ensure the struct write completes before updating hea
        head = n;
        return true;
    }

    bool q_pop(Event& out)
    {
        if (tail == head) return false; // q is empty
        out = q[tail];
        __DMB();
        tail = (tail + 1) % QSIZE;
        return true;
    }

    uint32_t last10 = 0;
    uint32_t last1s = 0;
}

namespace app
{
    void init(){
        ble::init();
        imu::init();
        ui::init();
        buttons::init();
        touch::init();
    }

    void post(const Event& e)
    {
        noInterrupts();
        q_push(e);
        interrupts();
    }

    void loop()
    {
        // periodic ticks, replays delay()
        uint32_t now = millis();
        if (now - last10 >= 10) { last10 = now; post(Event(EventType::TICK_10MS)); }
        if (now - last1s >= 1000) { last1s = now; imu::poll(); post(Event(EventType::TICK_1S)); }

        // process ANCS notification queue
        ble::poll();

        // poll button, will later be interrupt
        buttons::poll();
        touch::poll();
        if (touch::gesture() != touch::Gesture::NONE)
        {
            switch (touch::gesture()) 
            {
                case touch::Gesture::SINGLE_TAP:
                    post(Event(EventType::TOUCH_TAP,
                        touch::point().x, touch::point().y));
                        break;
                case touch::Gesture::SWIPE_UP:
                    post(Event(EventType::TOUCH_SWIPE_UP));
                    break;
                case touch::Gesture::SWIPE_DOWN:
                    post(Event(EventType::TOUCH_SWIPE_DOWN, touch::start_point().x, touch::start_point().y));
                    break;
                case touch::Gesture::SWIPE_LEFT:
                    post(Event(EventType::TOUCH_SWIPE_LEFT));
                    break;
                case touch::Gesture::SWIPE_RIGHT: {
                    auto start = touch::start_point();
                    post(Event(EventType::TOUCH_SWIPE_RIGHT, start.x, start.y));
                } break;
                case touch::Gesture::LONG_PRESS:
                    post(Event(EventType::TOUCH_LONG));
                    break;
                default: break;
            }
        }
        // sends events — MEDIA_* are intercepted for BLE before reaching ui
        Event e;
        while (q_pop(e))
        {
            switch (e.type)
            {
                case EventType::MEDIA_PREV:      ble::media_prev();      break;
                case EventType::MEDIA_PLAYPAUSE: ble::media_playpause(); break;
                case EventType::MEDIA_NEXT:      ble::media_next();      break;
                default:                         ui::handle(e);          break;
            }
        }

        // lets LVGL always run
        ui::tick();
    }
}