#include "ui.h"
#include <lvgl.h>
#include "../../include/config.h"

namespace
{
    enum class Screen {LOCKSCREEN, HEALTH, NOTIF, MUSIC, SETTINGS};
    Screen cur = Screen::LOCKSCREEN;

    void show(Screen s)
    {
        cur = s;
        lv_obj_clean(lv_scr_act());

        // placeholders
        lv_obj_t* label = lv_label_create(lv_scr_act());
        lv_label_set_text(label, 
        (s == Screen::LOCKSCREEN) ? "Lock Screen" :
        (s == Screen::HEALTH)     ? "Health" :
        (s == Screen::NOTIF)      ? "Notifications" :
        (s == Screen::MUSIC)      ? "Music" :
        (s == Screen::SETTINGS)     ? "Settings");
        lv_obj_center(label);
    }
}

namespace ui
{
    void init()
    {
        lv_init();
        // TODO: init display + register LVGL display driver
        // - create draw buffer (partial)
        // - set flush callback
        // - register driver

        show(Screen::LOCKSCREEN);
    }

    void tick()
    {
        lv_timer_handler();
    }

    void handle(const Event& e)
    {
        switch (e.type)
        {
            case EventType::BTN_SHORT:
            {
                // cycles screen for now
                Screen next = (cur == Screen::SETTINGS) ? Screen::LOCKSCREEN : Screen((int)cur + 1);
                show(next);
            } break;

            case EventType::BTN_LONG:
            {
                show(Screen::LOCKSCREEN);    
            } break;

            case EventType::TICK_1S:
            {
                // will updates time, step, etc.
            } break;

            default break;
        }
    }
}