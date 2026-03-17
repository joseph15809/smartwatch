#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "../hal/rtc.h"
#include "../hal/ble.h"
#include "../hal/imu.h"
#include "../app/app.h"
#include "screens/lockscreen.h"
#include "screens/music.h"
#include "screens/health.h"
#include "screens/homescreen.h"
#include "screens/settings.h"
#include "screens/notif.h"
#include "screens/messages.h"
#include "screens/message_thread.h"
#include "../../include/config.h"

namespace
{
    enum class Screen { LOCKSCREEN, HOMESCREEN, HEALTH, NOTIF, MUSIC, SETTINGS,
                        MESSAGES, MESSAGE_THREAD };
    Screen cur       = Screen::LOCKSCREEN;
    Screen prev      = Screen::HOMESCREEN;  // restored when swiping out of NOTIF

    // Persistent clock overlay
    lv_obj_t* s_clock_lbl = nullptr;

    // Persisted states
    NotifState    s_notif;
    MessagesState s_messages;
    ThreadState   s_thread;

    void set_clock_visible(bool v)
    {
        if (!s_clock_lbl) return;
        if (v) lv_obj_clear_flag(s_clock_lbl, LV_OBJ_FLAG_HIDDEN);
        else   lv_obj_add_flag (s_clock_lbl, LV_OBJ_FLAG_HIDDEN);
    }

    void destroy_current()
    {
        switch (cur)
        {
            case Screen::LOCKSCREEN:     lockscreen_destroy();  break;
            case Screen::HOMESCREEN:     homescreen_destroy();  break;
            case Screen::HEALTH:         health_destroy();      break;
            case Screen::MUSIC:          music_destroy();       break;
            case Screen::SETTINGS:       settings_destroy();    break;
            case Screen::NOTIF:          notif_destroy();       break;
            case Screen::MESSAGES:       messages_destroy();    break;
            case Screen::MESSAGE_THREAD: thread_destroy();      break;
            default: break;
        }
    }

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
            case Screen::SETTINGS:
                settings_create();
                break;
            case Screen::NOTIF:
                notif_create();
                notif_update(s_notif);
                break;
            case Screen::MESSAGES:
                messages_create();
                messages_update(s_messages);
                break;
            case Screen::MESSAGE_THREAD:
                thread_create(s_thread);
                break;
            default: break;
        }
    }

    // Show a screen, optionally recording a "back" destination for NOTIF
    void show(Screen s, bool record_prev = true)
    {
        if (record_prev && s == Screen::NOTIF && cur != Screen::NOTIF)
            prev = cur;   // remember where we came from (never record NOTIF as prev)

        destroy_current();
        lv_obj_clean(lv_scr_act());
        cur = s;
        create_screen(s);
        set_clock_visible(s != Screen::LOCKSCREEN);

        // Update clock text immediately
        if (s_clock_lbl)
        {
            rtc::DateTime dt = rtc::now();
            char buf[8];
            snprintf(buf, sizeof(buf), "%02u:%02u", dt.hour, dt.min);
            lv_label_set_text(s_clock_lbl, buf);
        }
    }

    Screen app_to_screen(AppId id)
    {
        switch (id)
        {
            case AppId::HEALTH:   return Screen::HEALTH;
            case AppId::MUSIC:    return Screen::MUSIC;
            case AppId::MESSAGES: return Screen::MESSAGES;
            case AppId::SETTINGS: return Screen::SETTINGS;
            default:              return Screen::HOMESCREEN;
        }
    }
}

namespace ui
{
    void init()
    {
        rtc::DateTime boot_time =
        {
            .year  = 2025,
            .month = 4,
            .day   = 6,
            .hour  = 9,
            .min   = 58,
            .sec   = 0,
            .wday  = 5
        };
        rtc::init(boot_time);

        // Persistent clock overlay on LVGL top layer
        s_clock_lbl = lv_label_create(lv_layer_top());
        lv_obj_set_style_text_color(s_clock_lbl, lv_color_white(), 0);
        lv_obj_set_style_text_font(s_clock_lbl, &lv_font_montserrat_16, 0);
        lv_obj_set_style_bg_color(s_clock_lbl, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(s_clock_lbl, LV_OPA_50, 0);
        lv_obj_set_style_radius(s_clock_lbl, 8, 0);
        lv_obj_set_style_pad_hor(s_clock_lbl, 8, 0);
        lv_obj_set_style_pad_ver(s_clock_lbl, 3, 0);
        lv_obj_align(s_clock_lbl, LV_ALIGN_TOP_MID, 0, 4);
        lv_label_set_text(s_clock_lbl, "00:00");

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
            // buttons 
            case EventType::BTN_SHORT: {
                if      (cur == Screen::MESSAGE_THREAD) show(Screen::MESSAGES);
                else if (cur == Screen::NOTIF)          show(prev);
                else                                    show(Screen::HOMESCREEN);
            } break;

            case EventType::BTN_LONG: {
                if (cur != Screen::LOCKSCREEN) show(Screen::LOCKSCREEN);
            } break;

            // ticks 
            case EventType::TICK_1S: {
                rtc::DateTime dt = rtc::now();
                if (cur == Screen::LOCKSCREEN) lockscreen_update(dt);
                if (s_clock_lbl)
                {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "%02u:%02u", dt.hour, dt.min);
                    lv_label_set_text(s_clock_lbl, buf);
                }
                if (cur == Screen::HEALTH)
                {
                    HealthState hs;
                    hs.heart_rate = 0;
                    hs.steps      = imu::steps();
                    hs.distance   = imu::distance_m();
                    health_update(hs);
                }
            } break;

            // BLE
            case EventType::BLE_TIME_SYNC: {
                rtc::DateTime dt = rtc::now();
                if (s_clock_lbl)
                {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "%02u:%02u", dt.hour, dt.min);
                    lv_label_set_text(s_clock_lbl, buf);
                }
                if (cur == Screen::LOCKSCREEN) lockscreen_update(dt);
            } break;

            case EventType::BLE_NOTIF: {
                s_notif = ble::notif_state();
                if (cur == Screen::NOTIF) notif_update(s_notif);
            } break;

            case EventType::BLE_MUSIC_META: {
                if (cur == Screen::MUSIC) music_update(ble::music_state());
            } break;

            case EventType::BLE_MSG_UPDATE: {
                s_messages = ble::messages_state();
                if (cur == Screen::MESSAGES) messages_update(s_messages);
            } break;

            case EventType::BLE_THREAD_UPDATE: {
                s_thread = ble::thread_state();
                if (cur == Screen::MESSAGE_THREAD) {
                    destroy_current();
                    lv_obj_clean(lv_scr_act());
                    cur = Screen::MESSAGE_THREAD;
                    thread_create(s_thread);
                    set_clock_visible(true);
                }
            } break;

            case EventType::BLE_MEDIA_STATE: {
                if (cur == Screen::MUSIC)
                {
                    // Use the full state from BLE (includes title/artist already set by MC)
                    MusicState ms = ble::music_state();
                    ms.elapsed  = (uint32_t)e.a;
                    ms.duration = (uint32_t)e.b;
                    music_update(ms);
                }
            } break;

            // touch: tap
            case EventType::TOUCH_TAP: {
                int16_t tx = (int16_t)e.a;
                int16_t ty = (int16_t)e.b;

                if (cur == Screen::HOMESCREEN)
                {
                    // ICON_SIZE=60, GAP=24, GRID_X=48, stride=84
                    for (int i = 0; i < 4; i++)
                    {
                        int col = i % 2;
                        int row = i / 2;
                        int x = 48 + col * (60 + 24);
                        int y = 48 + row * (60 + 24);
                        if (tx >= x && tx < x + 60 && ty >= y && ty < y + 60)
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
                else if (cur == Screen::NOTIF)
                {
                    // X button: x=190, y=34, size=30×26
                    if (tx >= 190 && ty >= 34 && ty < 60)
                    {
                        s_notif.count = 0;
                        notif_clear_all();
                    }
                    // Cards: scroll_cont starts at y=68
                    else if (ty >= 68)
                    {
                        int content_y = (ty - 68) + notif_scroll_y();
                        int idx = (content_y - 8) / 72;
                        if (idx >= 0 && idx < notif_count())
                            show(app_to_screen(notif_source(idx)));
                    }
                }
                else if (cur == Screen::MESSAGES)
                {
                    // Cards: scroll_cont starts at y=60
                    if (ty >= 60)
                    {
                        int content_y = (ty - 60) + messages_scroll_y();
                        int idx = (content_y - 8) / 72;
                        if (idx >= 0 && idx < messages_count())
                        {
                            const char* peer = messages_peer(idx);
                            // Use BLE thread state if it matches this peer,
                            // otherwise show an empty thread for this peer
                            const ThreadState& ble_thread = ble::thread_state();
                            if (strncmp(ble_thread.peer, peer, sizeof(s_thread.peer)) == 0)
                                s_thread = ble_thread;
                            else {
                                s_thread = ThreadState{};
                                strncpy(s_thread.peer, peer, sizeof(s_thread.peer) - 1);
                            }
                            show(Screen::MESSAGE_THREAD);
                        }
                    }
                }
                else if (cur == Screen::MUSIC)
                {
                    // Controls: y=158–210, container x=10–230
                    // PREV x<80, PLAY/PAUSE x<150, NEXT x>=150
                    if (ty >= 158 && ty < 210 && tx >= 10 && tx < 230)
                    {
                        if      (tx < 80)  app::post(Event(EventType::MEDIA_PREV));
                        else if (tx < 150) app::post(Event(EventType::MEDIA_PLAYPAUSE));
                        else               app::post(Event(EventType::MEDIA_NEXT));
                    }
                }
            } break;

            // touch: swipes
            case EventType::TOUCH_SWIPE_UP: {
                if (cur == Screen::NOTIF)
                {
                    // Swipe up leaves notifications, returns to previous screen
                    show(prev);
                }
                else if (cur == Screen::HEALTH)         health_scroll(72);
                else if (cur == Screen::SETTINGS)       settings_scroll(72);
                else if (cur == Screen::MESSAGES)       messages_scroll(72);
                else if (cur == Screen::MESSAGE_THREAD) thread_scroll(72);
            } break;

            case EventType::TOUCH_SWIPE_DOWN: {
                // Top-edge swipe always opens notifications
                if (e.b < 40)
                {
                    show(Screen::NOTIF);
                }
                else if (cur == Screen::HEALTH)         health_scroll(-72);
                else if (cur == Screen::SETTINGS)       settings_scroll(-72);
                else if (cur == Screen::NOTIF)          notif_scroll(-72);
                else if (cur == Screen::MESSAGES)       messages_scroll(-72);
                else if (cur == Screen::MESSAGE_THREAD) thread_scroll(-72);
            } break;

            case EventType::TOUCH_SWIPE_RIGHT: {
                if (cur == Screen::NOTIF)
                {
                    // e.b = start_y; cards start at y=68 in screen coords
                    int content_y = (e.b - 68) + notif_scroll_y();
                    int idx = (content_y - 8) / 72;
                    if (idx >= 0 && idx < notif_count())
                        notif_dismiss(idx);
                }
            } break;

            default: break;
        }
    }
}
