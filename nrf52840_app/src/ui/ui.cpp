#include "ui.h"
#include <lvgl.h>
#include "../../include/lv_conf.h"
#include "screens/lockscreen.h"
// later:
// #include "screens/health.h"
// #include "screens/notifications.h"
// #include "screens/music.h"
// #include "screens/settings.h"

namespace
{
    enum class Screen {LOCKSCREEN, HEALTH, NOTIF, MUSIC, SETTINGS};
    Screen cur = Screen::LOCKSCREEN;

void show(Screen s)
{
    cur = s;
    lv_obj_clean(lv_scr_act());   // wipe current screen

    switch (s)
    {
        case Screen::LOCKSCREEN:
            lockscreen_create();
            break;

        case Screen::HEALTH: {
            // health_create();
            lv_obj_t* label = lv_label_create(lv_scr_act());
            lv_label_set_text(label, "Health");
            lv_obj_center(label);
        } break;

        case Screen::NOTIF: {
            // notif_create();
            lv_obj_t* label = lv_label_create(lv_scr_act());
            lv_label_set_text(label, "Notifications");
            lv_obj_center(label);
        } break;

        case Screen::MUSIC: {
            // music_create();
            lv_obj_t* label = lv_label_create(lv_scr_act());
            lv_label_set_text(label, "Music");
            lv_obj_center(label);
        } break;

        case Screen::SETTINGS: {
            // settings_create();
            lv_obj_t* label = lv_label_create(lv_scr_act());
            lv_label_set_text(label, "Settings");
            lv_obj_center(label);
        } break;

        default:
            break;
    }
}
}

namespace ui
{
    void init()
    {
        // TODO: init display + register LVGL display driver
        // - create draw buffer (partial)
        // - set flush callback
        // - register driver
        // Create a quick test UI
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

            default: break;
        }
    }
}