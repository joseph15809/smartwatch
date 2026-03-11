#include <lvgl.h>
#include "ui.h"
#include "../hal/rtc.h"
#include "screens/lockscreen.h"
#include "screens/music.h"
#include "screens/health.h"
#include "screens/homescreen.h"
#include "../../include/config.h"

// later:
// #include "screens/health.h"
// #include "screens/notifications.h"
// #include "screens/music.h"
// #include "screens/settings.h"

namespace
{
    enum class Screen {LOCKSCREEN, HOMESCREEN, HEALTH, NOTIF, MUSIC, SETTINGS};
    Screen cur = Screen::LOCKSCREEN;

    void destroy_current()
    {
        switch (cur)
        {
            case Screen::LOCKSCREEN: lockscreen_destroy(); break;
            case Screen::HOMESCREEN: homescreen_destroy(); break;
            case Screen::HEALTH:     health_destroy();     break;
            case Screen::MUSIC:      music_destroy();      break;
            default: break;
        }
    }

    // build new screen after lv_obj_clean
    void create_screen(Screen s)
    {
        switch (s) 
        {
            case Screen::LOCKSCREEN:
                lockscreen_create();
                lockscreen_update(rtc::now());
                break;
            case Screen::HOMESCREEN:
                homescreen_create();
                break;
            case Screen::HEALTH:
                health_create();
                break;
            case Screen::MUSIC:
                music_create();
                break;
            case Screen::NOTIF: 
            {
                lv_obj_t* lbl = lv_label_create(lv_scr_act());
                lv_label_set_text(lbl, "Notifications");
                lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
                lv_obj_center(lbl);
            } break;
            case Screen::SETTINGS: 
            {
                lv_obj_t* lbl = lv_label_create(lv_scr_act());
                lv_label_set_text(lbl, "Settings");
                lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
                lv_obj_center(lbl);
            } break;
            default: break;
        }
    }

    void show(Screen s)
    {
        destroy_current();          // 1. null out pointers / stop timers
        lv_obj_clean(lv_scr_act()); // 2. free all LVGL objects
        cur = s;
        create_screen(s);           // 3. build new screen
    }

    Screen app_to_screen(AppId id) 
    {
        switch (id) 
        {
            case AppId::HEALTH:        return Screen::HEALTH;
            case AppId::MUSIC:         return Screen::MUSIC;
            case AppId::NOTIFICATIONS: return Screen::NOTIF;
            case AppId::SETTINGS:      return Screen::SETTINGS;
            default:                   return Screen::HOMESCREEN;
        }
    }
}

namespace ui
{
    void init()
    {
        // RTC: set a sane default time so the clock shows something before BLE sync.
        // Replace these values with a compile-time macro or BLE sync later.
        rtc::DateTime boot_time = 
        {
            .year  = 2025,
            .month = 4,
            .day   = 6,
            .hour  = 9,
            .min   = 58,
            .sec   = 0,
            .wday  = 5  // (0=Sun)
        };
        rtc::init(boot_time);

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
            case EventType::BTN_SHORT: {
                show(Screen::HOMESCREEN);
            } break;

            case EventType::BTN_LONG: {
                if (cur != Screen::LOCKSCREEN) 
                {
                    show(Screen::LOCKSCREEN);
                }
            } break;

            case EventType::TICK_1S: {
                rtc::DateTime dt = rtc::now();
                if (cur == Screen::LOCKSCREEN) lockscreen_update(dt);
            } break;

            case EventType::BLE_MEDIA_STATE: {
                if (cur == Screen::MUSIC) {
                    MusicState ms;
                    ms.elapsed  = (uint32_t)e.a;
                    ms.duration = (uint32_t)e.b;
                    music_update(ms);
                }
            } break;
            case EventType::TOUCH_TAP: {
                if (cur == Screen::HOMESCREEN) 
                {
                    int16_t tx = (int16_t)e.a;
                    int16_t ty = (int16_t)e.b;

                    for (int i = 0; i < 4; i++)
                    {
                        int col = i % 2;
                        int row = i / 2;
                        int x = 38 + col * (70 + 24);  // GRID_X=38, ICON_SIZE=70, GAP=24
                        int y = 38 + row * (70 + 24);

                        if (tx >= x && tx < x + 70 && ty >= y && ty < y + 70)
                        {
                            AppId id = homescreen_selected_at(i);
                            if (id != AppId::COUNT) show(app_to_screen(id));
                            break;
                        }
                    }
                } 
                else if (cur == Screen::LOCKSCREEN) 
                {
                    show(Screen::HOMESCREEN);
                }
            } break;

            case EventType::TOUCH_SWIPE_UP: {
                if (cur == Screen::HEALTH) health_scroll(-72);
            } break;

            case EventType::TOUCH_SWIPE_DOWN: {
                if (cur == Screen::HEALTH)
                {
                    health_scroll(72);
                }
                else if (e.b < 40)
                {
                    show(Screen::NOTIF);
                }
            } break;

            default: break;
        }
    }
}