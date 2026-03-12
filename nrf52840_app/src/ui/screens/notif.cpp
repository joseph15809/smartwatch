#include <lvgl.h>
#include "notif.h"
#include "../../../include/config.h"

namespace
{
    bool       s_active      = false;
    int        s_scroll_y    = 0;
    lv_obj_t*  s_scroll_cont = nullptr;
    lv_obj_t*  s_empty_lbl   = nullptr;
    NotifState s_state;

    static const lv_color_t kCardBg = LV_COLOR_MAKE(0x1E, 0x1E, 0x1E);

    int safe_y() { int y = (DISP_VER - 240) / 2; return y < 0 ? 0 : y; }

    int max_scroll()
    {
        if (s_state.count == 0) return 0;
        int content = 8 + s_state.count * 72 + 72;  // pad_bottom=72
        int ms = content - 172;
        return ms > 0 ? ms : 0;
    }

    void rebuild_cards()
    {
        lv_obj_clean(s_scroll_cont);

        if (s_state.count == 0)
        {
            lv_obj_clear_flag(s_empty_lbl, LV_OBJ_FLAG_HIDDEN);
            return;
        }
        lv_obj_add_flag(s_empty_lbl, LV_OBJ_FLAG_HIDDEN);

        for (int i = 0; i < s_state.count; i++)
        {
            const NotifEntry& n = s_state.entries[i];
            lv_obj_t* card = lv_obj_create(s_scroll_cont);
            lv_obj_set_size(card, 210, 62);
            lv_obj_set_pos(card, 15, 8 + i * 72);
            lv_obj_set_style_bg_color(card, kCardBg, 0);
            lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(card, 0, 0);
            lv_obj_set_style_radius(card, 12, 0);
            lv_obj_set_style_pad_left(card, 14, 0);
            lv_obj_set_style_pad_top(card, 8, 0);
            lv_obj_set_style_pad_right(card, 14, 0);
            lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t* title = lv_label_create(card);
            lv_label_set_text(title, n.title);
            lv_obj_set_style_text_color(title, lv_color_white(), 0);
            lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
            lv_obj_set_pos(title, 0, 0);

            lv_obj_t* body = lv_label_create(card);
            lv_label_set_text(body, n.body);
            lv_obj_set_style_text_color(body, LV_COLOR_MAKE(0xAA, 0xAA, 0xAA), 0);
            lv_obj_set_style_text_font(body, &lv_font_montserrat_14, 0);
            lv_obj_set_width(body, 180);
            lv_label_set_long_mode(body, LV_LABEL_LONG_DOT);
            lv_obj_set_pos(body, 0, 22);
        }
    }
}

void notif_create()
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

    // "X" clear-all button — top right, aligned with other screen titles (y=34)
    lv_obj_t* btn = lv_obj_create(root);
    lv_obj_set_size(btn, 30, 26);
    lv_obj_set_pos(btn, 190, 34);
    lv_obj_set_style_bg_color(btn, LV_COLOR_MAKE(0x40, 0x20, 0x20), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t* btn_lbl = lv_label_create(btn);
    lv_label_set_text(btn_lbl, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(btn_lbl, LV_COLOR_MAKE(0xFF, 0x60, 0x60), 0);
    lv_obj_set_style_text_font(btn_lbl, &lv_font_montserrat_16, 0);
    lv_obj_center(btn_lbl);

    // Scrollable card container (y=68: btn_y=34 + btn_h=26 + gap=8)
    lv_obj_t* sc = s_scroll_cont = lv_obj_create(root);
    lv_obj_set_size(sc, 240, 172);
    lv_obj_set_pos(sc, 0, 68);
    lv_obj_set_style_bg_opa(sc, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(sc, 0, 0);
    lv_obj_set_style_pad_top(sc, 0, 0);
    lv_obj_set_style_pad_left(sc, 0, 0);
    lv_obj_set_style_pad_right(sc, 0, 0);
    lv_obj_set_style_pad_bottom(sc, 72, 0);

    // Empty state
    s_empty_lbl = lv_label_create(root);
    lv_label_set_text(s_empty_lbl, "No Notifications");
    lv_obj_set_style_text_color(s_empty_lbl, LV_COLOR_MAKE(0x77, 0x77, 0x77), 0);
    lv_obj_set_style_text_font(s_empty_lbl, &lv_font_montserrat_16, 0);
    lv_obj_align(s_empty_lbl, LV_ALIGN_CENTER, 0, 20);
    lv_obj_clear_flag(s_empty_lbl, LV_OBJ_FLAG_HIDDEN);  // shown until update() called
}

void notif_update(const NotifState& s)
{
    if (!s_active) return;
    s_state    = s;
    s_scroll_y = 0;
    lv_obj_scroll_to_y(s_scroll_cont, 0, LV_ANIM_OFF);
    rebuild_cards();
}

void notif_scroll(int dy)
{
    if (!s_active || !s_scroll_cont) return;
    int ms = max_scroll();
    s_scroll_y += dy;
    if (s_scroll_y < 0)  s_scroll_y = 0;
    if (s_scroll_y > ms) s_scroll_y = ms;
    lv_obj_scroll_to_y(s_scroll_cont, s_scroll_y, LV_ANIM_ON);
}

void notif_dismiss(int idx)
{
    if (!s_active || idx < 0 || idx >= s_state.count) return;
    for (int i = idx; i < s_state.count - 1; i++)
        s_state.entries[i] = s_state.entries[i + 1];
    s_state.count--;
    int ms = max_scroll();
    if (s_scroll_y > ms) s_scroll_y = ms;
    lv_obj_scroll_to_y(s_scroll_cont, s_scroll_y, LV_ANIM_OFF);
    rebuild_cards();
}

void notif_clear_all()
{
    if (!s_active) return;
    s_state.count = 0;
    s_scroll_y    = 0;
    lv_obj_scroll_to_y(s_scroll_cont, 0, LV_ANIM_OFF);
    rebuild_cards();
}

int   notif_count()         { return s_active ? (int)s_state.count : 0; }
AppId notif_source(int idx) { return s_state.entries[idx].source; }
int   notif_scroll_y()      { return s_scroll_y; }

void notif_destroy()
{
    s_active      = false;
    s_scroll_y    = 0;
    s_scroll_cont = nullptr;
    s_empty_lbl   = nullptr;
    s_state       = NotifState{};
}
