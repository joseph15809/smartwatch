#include <lvgl.h>
#include <stdio.h>
#include "settings.h"
#include "../../../include/config.h"

namespace
{
    bool      s_active     = false;
    int       s_scroll_y   = 0;

    lv_obj_t* s_scroll_cont  = nullptr;

    // Value labels updated by settings_update()
    lv_obj_t* bright_val  = nullptr;
    lv_obj_t* bt_val      = nullptr;
    lv_obj_t* dnd_val     = nullptr;
    lv_obj_t* bat_val     = nullptr;
    lv_obj_t* ver_val     = nullptr;

    // Card colours
    static const lv_color_t kColorBright = LV_COLOR_MAKE(0xFF, 0xB0, 0x30); // amber
    static const lv_color_t kColorBT     = LV_COLOR_MAKE(0x50, 0xB0, 0xFF); // blue
    static const lv_color_t kColorDND    = LV_COLOR_MAKE(0xB0, 0x60, 0xFF); // purple
    static const lv_color_t kColorBat    = LV_COLOR_MAKE(0x50, 0xE0, 0x90); // green
    static const lv_color_t kColorAbout  = LV_COLOR_MAKE(0xAA, 0xAA, 0xAA); // gray

    static const lv_color_t kCardBg = LV_COLOR_MAKE(0x1E, 0x1E, 0x1E);

    lv_obj_t* make_card(lv_obj_t* parent, int y,
                        const char* symbol, lv_color_t sym_color,
                        const char* heading, const char* value)
    {
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

        // Value label
        lv_obj_t* val_lbl = lv_label_create(card);
        lv_label_set_text(val_lbl, value);
        lv_obj_set_style_text_color(val_lbl, lv_color_white(), 0);
        lv_obj_set_style_text_font(val_lbl, &lv_font_montserrat_20, 0);
        lv_obj_set_pos(val_lbl, 0, 32);

        return val_lbl;
    }

    int safe_y() {
        int y = (DISP_VER - 240) / 2;
        return (y < 0) ? 0 : y;
    }
}

void settings_create()
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

    // Screen title
    lv_obj_t* title = lv_label_create(root);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 34);

    // Scrollable card container
    lv_obj_t* scroll_cont = s_scroll_cont = lv_obj_create(root);
    lv_obj_set_size(scroll_cont, 240, 180);
    lv_obj_set_pos(scroll_cont, 0, 60);
    lv_obj_set_style_bg_opa(scroll_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(scroll_cont, 0, 0);
    lv_obj_set_style_pad_top(scroll_cont, 0, 0);
    lv_obj_set_style_pad_left(scroll_cont, 0, 0);
    lv_obj_set_style_pad_right(scroll_cont, 0, 0);
    lv_obj_set_style_pad_bottom(scroll_cont, 72, 0);

    // Five setting cards — y: 8, 80, 152, 224, 296
    // content bottom = 296+62 = 358  -> MAX_SCROLL = 358-202 = 156
    bright_val = make_card(scroll_cont,   8, LV_SYMBOL_IMAGE,    kColorBright, "Brightness",       "80%");
    bt_val     = make_card(scroll_cont,  80, LV_SYMBOL_BLUETOOTH,kColorBT,     "Bluetooth",         "Off");
    dnd_val    = make_card(scroll_cont, 152, LV_SYMBOL_BELL,     kColorDND,    "Do Not Disturb",    "Off");
    bat_val    = make_card(scroll_cont, 224, LV_SYMBOL_CHARGE,   kColorBat,    "Battery",           "0%");
    ver_val    = make_card(scroll_cont, 296, LV_SYMBOL_SETTINGS, kColorAbout,  "About",             "v0.1.0");
}

void settings_update(const SettingsState& s)
{
    if (!s_active) return;

    char buf[16];

    snprintf(buf, sizeof(buf), "%u%%", s.brightness);
    lv_label_set_text(bright_val, buf);

    lv_label_set_text(bt_val,  s.bluetooth ? "On" : "Off");
    lv_label_set_text(dnd_val, s.dnd       ? "On" : "Off");

    snprintf(buf, sizeof(buf), "%u%%", s.battery);
    lv_label_set_text(bat_val, buf);

    lv_label_set_text(ver_val, s.fw_ver ? s.fw_ver : "v0.1.0");
}

void settings_scroll(int dy)
{
    if (!s_active || !s_scroll_cont) return;
    const int MAX_SCROLL = 250;  // content 358 + pad_bottom 72 - container 180
    s_scroll_y += dy;
    if (s_scroll_y < 0)          s_scroll_y = 0;
    if (s_scroll_y > MAX_SCROLL) s_scroll_y = MAX_SCROLL;
    lv_obj_scroll_to_y(s_scroll_cont, s_scroll_y, LV_ANIM_ON);
}

void settings_destroy()
{
    s_active     = false;
    s_scroll_y   = 0;
    s_scroll_cont = nullptr;
    bright_val   = nullptr;
    bt_val       = nullptr;
    dnd_val      = nullptr;
    bat_val      = nullptr;
    ver_val      = nullptr;
}
