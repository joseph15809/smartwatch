#include <lvgl.h>
#include "messages.h"
#include "../../../include/config.h"

namespace
{
    bool          s_active      = false;
    int           s_scroll_y    = 0;
    lv_obj_t*     s_scroll_cont = nullptr;
    lv_obj_t*     s_empty_lbl   = nullptr;
    MessagesState s_state;

    static const lv_color_t kCardBg = LV_COLOR_MAKE(0x1E, 0x1E, 0x1E);
    static const lv_color_t kUnread = LV_COLOR_MAKE(0x20, 0xC0, 0xA0);  // teal

    int safe_y() { int y = (DISP_VER - 240) / 2; return y < 0 ? 0 : y; }

    int max_scroll()
    {
        if (s_state.count == 0) return 0;
        int content = 8 + s_state.count * 72 + 72;  // pad_bottom=72
        int ms = content - 180;
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
            const Conversation& c = s_state.convs[i];
            lv_obj_t* card = lv_obj_create(s_scroll_cont);
            lv_obj_set_size(card, 210, 62);
            lv_obj_set_pos(card, 15, 8 + i * 72);
            lv_obj_set_style_bg_color(card, kCardBg, 0);
            lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(card, c.unread ? 2 : 0, 0);
            lv_obj_set_style_border_color(card, kUnread, 0);
            lv_obj_set_style_border_opa(card, LV_OPA_COVER, 0);
            lv_obj_set_style_radius(card, 12, 0);
            lv_obj_set_style_pad_left(card, 14, 0);
            lv_obj_set_style_pad_top(card, 8, 0);
            lv_obj_set_style_pad_right(card, 14, 0);
            lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

            // Peer name
            lv_obj_t* name = lv_label_create(card);
            lv_label_set_text(name, c.peer);
            lv_obj_set_style_text_color(name, lv_color_white(), 0);
            lv_obj_set_style_text_font(name, &lv_font_montserrat_14, 0);
            lv_obj_set_pos(name, 0, 0);

            // Message preview
            lv_obj_t* preview = lv_label_create(card);
            lv_label_set_text(preview, c.preview);
            lv_obj_set_style_text_color(preview, LV_COLOR_MAKE(0xAA, 0xAA, 0xAA), 0);
            lv_obj_set_style_text_font(preview, &lv_font_montserrat_14, 0);
            lv_obj_set_width(preview, 180);
            lv_label_set_long_mode(preview, LV_LABEL_LONG_DOT);
            lv_obj_set_pos(preview, 0, 22);
        }
    }
}

void messages_create()
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

    // Title
    lv_obj_t* title = lv_label_create(root);
    lv_label_set_text(title, "Messages");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 34);

    // Scrollable contact list
    lv_obj_t* sc = s_scroll_cont = lv_obj_create(root);
    lv_obj_set_size(sc, 240, 180);
    lv_obj_set_pos(sc, 0, 60);
    lv_obj_set_style_bg_opa(sc, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(sc, 0, 0);
    lv_obj_set_style_pad_top(sc, 0, 0);
    lv_obj_set_style_pad_left(sc, 0, 0);
    lv_obj_set_style_pad_right(sc, 0, 0);
    lv_obj_set_style_pad_bottom(sc, 72, 0);

    // Empty state
    s_empty_lbl = lv_label_create(root);
    lv_label_set_text(s_empty_lbl, "No Messages");
    lv_obj_set_style_text_color(s_empty_lbl, LV_COLOR_MAKE(0x77, 0x77, 0x77), 0);
    lv_obj_set_style_text_font(s_empty_lbl, &lv_font_montserrat_16, 0);
    lv_obj_align(s_empty_lbl, LV_ALIGN_CENTER, 0, 20);
    lv_obj_clear_flag(s_empty_lbl, LV_OBJ_FLAG_HIDDEN);  // shown until update() populates list
}

void messages_update(const MessagesState& s)
{
    if (!s_active) return;
    s_state    = s;
    s_scroll_y = 0;
    lv_obj_scroll_to_y(s_scroll_cont, 0, LV_ANIM_OFF);
    rebuild_cards();
}

void messages_scroll(int dy)
{
    if (!s_active || !s_scroll_cont) return;
    int ms = max_scroll();
    s_scroll_y += dy;
    if (s_scroll_y < 0)  s_scroll_y = 0;
    if (s_scroll_y > ms) s_scroll_y = ms;
    lv_obj_scroll_to_y(s_scroll_cont, s_scroll_y, LV_ANIM_ON);
}

int         messages_count()      { return s_active ? (int)s_state.count : 0; }
int         messages_scroll_y()   { return s_scroll_y; }
const char* messages_peer(int idx){ return s_state.convs[idx].peer; }

void messages_destroy()
{
    s_active      = false;
    s_scroll_y    = 0;
    s_scroll_cont = nullptr;
    s_empty_lbl   = nullptr;
    s_state       = MessagesState{};
}
