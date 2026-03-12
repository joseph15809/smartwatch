#include <lvgl.h>
#include <stdio.h>
#include "homescreen.h"
#include "../../../include/config.h"

namespace 
{

    constexpr int COLS = 2;
    constexpr int ROWS = 2;
    constexpr int TOTAL_ICONS = COLS * ROWS; // 4
    constexpr int ICON_SIZE = 60;
    constexpr int GAP = 24;
    constexpr int GRID_W = COLS * ICON_SIZE + (COLS - 1) * GAP; // 200
    constexpr int GRID_H = ROWS * ICON_SIZE + (ROWS - 1) * GAP; // 200
    constexpr int GRID_X = (240 - GRID_W) / 2; // 20
    constexpr int GRID_Y = (240 - GRID_H) / 2; // 20

    struct AppEntry 
    {
        AppId       id;
        const char* symbol;
        const char* label;
        lv_color_t  color;
    };

    static const AppEntry kApps[TOTAL_ICONS] = {
        { AppId::HEALTH,        LV_SYMBOL_EYE_OPEN, "Health",   LV_COLOR_MAKE(0x30, 0xB0, 0x60) },
        { AppId::MUSIC,         LV_SYMBOL_AUDIO,    "Music",    LV_COLOR_MAKE(0xE0, 0x60, 0x30) },
        { AppId::MESSAGES,      LV_SYMBOL_CALL,     "Messages", LV_COLOR_MAKE(0x20, 0xC0, 0xA0) },
        { AppId::SETTINGS,      LV_SYMBOL_SETTINGS, "Settings", LV_COLOR_MAKE(0x70, 0x70, 0x70) },
    };

    bool      s_active              = false;
    lv_obj_t* s_circles[TOTAL_ICONS] = {};

    void set_highlight(int idx, bool selected) 
    {
        if (!s_circles[idx]) return;
        if (selected)
        {
            lv_obj_set_style_border_color(s_circles[idx], lv_color_white(), 0);
            lv_obj_set_style_border_width(s_circles[idx], 3, 0);
            lv_obj_set_style_border_opa(s_circles[idx], LV_OPA_COVER, 0);
        } else
        {
            lv_obj_set_style_border_width(s_circles[idx], 0, 0);
        }
    }

}

void homescreen_create()
{
    s_active = true;

    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    for (int i = 0; i < TOTAL_ICONS; i++) 
    {
        int col = i % COLS;
        int row = i / COLS;
        int x   = GRID_X + col * (ICON_SIZE + GAP);
        int y   = GRID_Y + row * (ICON_SIZE + GAP);

        const AppEntry& app = kApps[i];

        // Circle
        lv_obj_t* circle = lv_obj_create(scr);
        lv_obj_set_size(circle, ICON_SIZE, ICON_SIZE);
        lv_obj_set_pos(circle, x, y);
        lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(circle, app.color, 0);
        lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(circle, 0, 0);
        lv_obj_set_style_pad_all(circle, 0, 0);
        lv_obj_clear_flag(circle, LV_OBJ_FLAG_SCROLLABLE);
        s_circles[i] = circle;

        // Symbol centered in circle
        lv_obj_t* sym = lv_label_create(circle);
        lv_label_set_text(sym, app.symbol);
        lv_obj_set_style_text_color(sym, lv_color_white(), 0);
        lv_obj_set_style_text_font(sym, &lv_font_montserrat_20, 0);
        lv_obj_center(sym);

        // App name below circle
        lv_obj_t* name = lv_label_create(scr);
        lv_label_set_text(name, app.label);
        lv_obj_set_style_text_font(name, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(name, LV_COLOR_MAKE(0xCC, 0xCC, 0xCC), 0);
        lv_obj_set_width(name, ICON_SIZE);
        lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_pos(name, x, y + ICON_SIZE + 2);
    }
}

AppId homescreen_selected_at(int idx)
{
    if (idx < 0 || idx >= TOTAL_ICONS) return AppId::COUNT;
    return kApps[idx].id;
}

void homescreen_destroy()
{
    s_active = false;
    for (int i = 0; i < TOTAL_ICONS; i++) 
    {
        s_circles[i] = nullptr;
    }
}