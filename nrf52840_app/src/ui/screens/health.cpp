#include <lvgl.h>
#include <stdio.h>
#include "health.h"
#include "../../../include/config.h"

namespace
{
    bool s_active = false;

    // value labels — updated by health_update()
    lv_obj_t* hr_val      = nullptr;
    lv_obj_t* steps_val   = nullptr;
    lv_obj_t* dist_val    = nullptr;
    lv_obj_t* s_scroll_cont = nullptr;

    static const lv_color_t kColorHR   = LV_COLOR_MAKE(0xE0, 0x50, 0x50); // red
    static const lv_color_t kColorStep = LV_COLOR_MAKE(0x50, 0xB0, 0xFF); // blue
    static const lv_color_t kColorDist = LV_COLOR_MAKE(0x50, 0xE0, 0x90); // green

    static const lv_color_t kCardBg    = LV_COLOR_MAKE(0x1E, 0x1E, 0x1E); // dark card

    lv_obj_t* make_card(lv_obj_t* parent, int y,
                        const char* symbol, lv_color_t sym_color,
                        const char* heading, const char* value)
    {
        // Card background
        lv_obj_t* card = lv_obj_create(parent);
        lv_obj_set_size(card, 210, 62);
        lv_obj_set_pos(card, 15, y);
        lv_obj_set_style_bg_color(card, kCardBg, 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(card, 0, 0);
        lv_obj_set_style_radius(card, 12, 0);
        lv_obj_set_style_pad_left(card, 14, 0);
        lv_obj_set_style_pad_top(card, 10, 0);
        lv_obj_set_style_pad_right(card, 14, 0);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

        // Top row: symbol + heading
        lv_obj_t* row = lv_obj_create(card);
        lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_pos(row, 0, 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(row, 6, 0);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* sym_lbl = lv_label_create(row);
        lv_label_set_text(sym_lbl, symbol);
        lv_obj_set_style_text_color(sym_lbl, sym_color, 0);
        lv_obj_set_style_text_font(sym_lbl, &lv_font_montserrat_16, 0);

        lv_obj_t* head_lbl = lv_label_create(row);
        lv_label_set_text(head_lbl, heading);
        lv_obj_set_style_text_color(head_lbl, LV_COLOR_MAKE(0xAA, 0xAA, 0xAA), 0);
        lv_obj_set_style_text_font(head_lbl, &lv_font_montserrat_14, 0);

        // Value label (large, below the row)
        lv_obj_t* val_lbl = lv_label_create(card);
        lv_label_set_text(val_lbl, value);
        lv_obj_set_style_text_color(val_lbl, lv_color_white(), 0);
        lv_obj_set_style_text_font(val_lbl, &lv_font_montserrat_20, 0);
        lv_obj_set_pos(val_lbl, 0, 32);

        return val_lbl; // caller stores this for updates
    }

    int safe_y() {
        int y = (DISP_VER - 240) / 2;
        return (y < 0) ? 0 : y;
    }
}

void health_create()
{
    s_active = true;

    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    // Safe 240x240 root
    lv_obj_t* root = lv_obj_create(scr);
    lv_obj_set_size(root, 240, 240);
    lv_obj_set_pos(root, 0, safe_y());
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    // Screen title (centered)
    lv_obj_t* title = lv_label_create(root);
    lv_label_set_text(title, "Activity");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Scrollable container for metric cards
    lv_obj_t* scroll_cont = s_scroll_cont = lv_obj_create(root);
    lv_obj_set_size(scroll_cont, 240, 202);
    lv_obj_set_pos(scroll_cont, 0, 38);
    lv_obj_set_style_bg_opa(scroll_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(scroll_cont, 0, 0);
    lv_obj_set_style_pad_all(scroll_cont, 0, 0);

    // Three metric cards inside scroll container
    hr_val    = make_card(scroll_cont,   8, LV_SYMBOL_EYE_OPEN, kColorHR,   "Heart Rate",     "--  BPM");
    steps_val = make_card(scroll_cont,  80, LV_SYMBOL_SHUFFLE,  kColorStep, "Total Steps",    "0");
    dist_val  = make_card(scroll_cont, 152, LV_SYMBOL_CHARGE,   kColorDist, "Total Distance", "0.0 KM");
}

void health_create(const HealthState& s)
{
    if (!s_active) return;

    char    buf[16];
    // heart rate will show -- until sensor arrives
    if(s.heart_rate == 0)
    {
        lv_label_set_text(hr_val, "-- BPM");
    }
    else
    {
        sniprintf(buf, sizeof(buf), "%u BPM", s.heart_rate);
        lv_label_set_text(hr_val, buf);
    }

    snprintf(buf, sizeof(buf), "%lu", (unsigned long)s.steps);
    lv_label_set_text(steps_val, buf);
    
    uint32_t mi_int = s.distance / 1609.34;
    uint32_t mi_frac = (s.distance % 1609) / 100;
    snprintf(buf, sizeof(buf), "%lu.%lu MI", (unsigned long)mi_int, (unsigned long)mi_frac);
    lv_label_set_text(dist_val, buf);
}

void health_scroll(int dy)
{
    if (!s_active || !s_scroll_cont) return;
    lv_obj_scroll_by(s_scroll_cont, 0, dy, LV_ANIM_ON);
}

void health_destroy()
{
    s_active      = false;
    hr_val        = nullptr;
    steps_val     = nullptr;
    dist_val      = nullptr;
    s_scroll_cont = nullptr;
}