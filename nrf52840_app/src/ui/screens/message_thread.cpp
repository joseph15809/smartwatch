#include <lvgl.h>
#include "message_thread.h"
#include "../../../include/config.h"

namespace
{
    bool       s_active      = false;
    int        s_scroll_y    = 0;
    int        s_max_scroll  = 0;
    lv_obj_t*  s_scroll_cont = nullptr;

    static const lv_color_t kSentBg = LV_COLOR_MAKE(0x20, 0xC0, 0xA0);  // teal (sent)
    static const lv_color_t kRecvBg = LV_COLOR_MAKE(0x2A, 0x2A, 0x2A);  // dark (received)

    // Fixed-height bubble: 44px height, 8px gap → 52px stride
    static constexpr int BUBBLE_H  = 44;
    static constexpr int BUBBLE_GAP = 8;
    static constexpr int STRIDE     = BUBBLE_H + BUBBLE_GAP;
    static constexpr int BUBBLE_W   = 190;
    static constexpr int CONT_H     = 180;

    int safe_y() { int y = (DISP_VER - 240) / 2; return y < 0 ? 0 : y; }
}

void thread_create(const ThreadState& s)
{
    s_active   = true;
    s_scroll_y = 0;

    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t* root = lv_obj_create(scr);
    lv_obj_set_size(root, 240, 240);
    lv_obj_set_pos(root, 0, safe_y());
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    // Peer name title
    lv_obj_t* title = lv_label_create(root);
    lv_label_set_text(title, s.peer);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 34);

    // Scrollable bubble area
    lv_obj_t* sc = s_scroll_cont = lv_obj_create(root);
    lv_obj_set_size(sc, 240, 180);
    lv_obj_set_pos(sc, 0, 60);
    lv_obj_set_style_bg_opa(sc, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(sc, 0, 0);
    lv_obj_set_style_pad_top(sc, 0, 0);
    lv_obj_set_style_pad_left(sc, 0, 0);
    lv_obj_set_style_pad_right(sc, 0, 0);
    lv_obj_set_style_pad_bottom(sc, 72, 0);

    for (int i = 0; i < s.count; i++)
    {
        const ThreadMessage& m = s.messages[i];
        int y = 8 + i * STRIDE;
        int x = m.sent ? (240 - BUBBLE_W - 8) : 8;

        lv_obj_t* bubble = lv_obj_create(sc);
        lv_obj_set_size(bubble, BUBBLE_W, BUBBLE_H);
        lv_obj_set_pos(bubble, x, y);
        lv_obj_set_style_bg_color(bubble, m.sent ? kSentBg : kRecvBg, 0);
        lv_obj_set_style_bg_opa(bubble, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(bubble, 0, 0);
        lv_obj_set_style_radius(bubble, 10, 0);
        lv_obj_set_style_pad_left(bubble, 10, 0);
        lv_obj_set_style_pad_top(bubble, 6, 0);
        lv_obj_set_style_pad_right(bubble, 10, 0);
        lv_obj_clear_flag(bubble, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* txt = lv_label_create(bubble);
        lv_label_set_text(txt, m.text);
        lv_obj_set_style_text_color(txt, m.sent ? lv_color_black() : lv_color_white(), 0);
        lv_obj_set_style_text_font(txt, &lv_font_montserrat_14, 0);
        lv_obj_set_width(txt, BUBBLE_W - 20);
        lv_label_set_long_mode(txt, LV_LABEL_LONG_DOT);
    }

    // Compute max scroll for this message count
    int content = 8 + s.count * STRIDE + 72;  // pad_bottom=72
    s_max_scroll = content > CONT_H ? content - CONT_H : 0;

    // Start scrolled to the bottom so the newest messages are visible
    s_scroll_y = s_max_scroll;
    lv_obj_scroll_to_y(s_scroll_cont, s_scroll_y, LV_ANIM_OFF);
}

void thread_scroll(int dy)
{
    if (!s_active || !s_scroll_cont) return;
    s_scroll_y += dy;
    if (s_scroll_y < 0)             s_scroll_y = 0;
    if (s_scroll_y > s_max_scroll)  s_scroll_y = s_max_scroll;
    lv_obj_scroll_to_y(s_scroll_cont, s_scroll_y, LV_ANIM_ON);
}

void thread_destroy()
{
    s_active      = false;
    s_scroll_y    = 0;
    s_max_scroll  = 0;
    s_scroll_cont = nullptr;
}
