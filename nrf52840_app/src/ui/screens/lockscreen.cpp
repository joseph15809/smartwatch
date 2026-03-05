#include <lvgl.h>
#include <cstdio>
#include "../../../include/config.h"

static lv_obj_t* hour_label;
static lv_obj_t* min_label;
static lv_obj_t* date1_label;
static lv_obj_t* date2_label;
static lv_obj_t* temp_label;
static lv_obj_t* weather_icon;

static int safe_y() {
    // Dev screen is 240x280
    // Round screen later is 240x240
    int y = (DISP_VER - 240) / 2;
    return (y < 0) ? 0 : y;
}

void lockscreen_create()
{
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    // ---- Safe watch area: always 240x240, centered vertically if needed ----
    lv_obj_t* root = lv_obj_create(scr);
    lv_obj_set_size(root, 240, 240);
    lv_obj_set_pos(root, 0, safe_y());

    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);

    // Root layout: two columns
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(root,
                          LV_FLEX_ALIGN_START,   // main axis (row): left to right
                          LV_FLEX_ALIGN_START,   // cross axis: top
                          LV_FLEX_ALIGN_START);  // track align
    lv_obj_set_style_pad_column(root, 10, 0);
    lv_obj_set_style_pad_row(root, 10, 0);

    // ---- Left column: hour/min stacked ----
    lv_obj_t* left = lv_obj_create(root);
    lv_obj_set_size(left, 120, 240);

    lv_obj_set_style_bg_opa(left, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left, 0, 0);

    // padding to match your mock spacing
    lv_obj_set_style_pad_left(left, 18, 0);
    lv_obj_set_style_pad_top(left, 28, 0);
    lv_obj_set_style_pad_row(left, 18, 0);

    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left,
                          LV_FLEX_ALIGN_START,   // stack top-down
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START);

    hour_label = lv_label_create(left);
    lv_label_set_text(hour_label, "12");
    lv_obj_set_style_text_color(hour_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(hour_label, &lv_font_montserrat_48, 0);

    min_label = lv_label_create(left);
    lv_label_set_text(min_label, "04");
    lv_obj_set_style_text_color(min_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(min_label, &lv_font_montserrat_48, 0);

    // ---- Right column: date + weather ----
    lv_obj_t* right = lv_obj_create(root);
    lv_obj_set_size(right, 120, 240);

    lv_obj_set_style_bg_opa(right, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right, 0, 0);

    lv_obj_set_style_pad_top(right, 40, 0);
    lv_obj_set_style_pad_row(right, 10, 0);

    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START);

    date1_label = lv_label_create(right);
    lv_label_set_text(date1_label, "28, February");
    lv_obj_set_style_text_color(date1_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(date1_label, &lv_font_montserrat_16, 0);

    date2_label = lv_label_create(right);
    lv_label_set_text(date2_label, "Friday");
    lv_obj_set_style_text_color(date2_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(date2_label, &lv_font_montserrat_20, 0);

    // Weather row (icon + temp)
    lv_obj_t* weather = lv_obj_create(right);
    lv_obj_set_size(weather, 90, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(weather, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(weather, 0, 0);
    lv_obj_set_style_pad_all(weather, 0, 0);

    lv_obj_set_flex_flow(weather, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(weather,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(weather, 10, 0);

    weather_icon = lv_label_create(weather);
    lv_label_set_text(weather_icon, LV_SYMBOL_UP); // placeholder
    lv_obj_set_style_text_color(weather_icon, lv_color_white(), 0);
    lv_obj_set_style_text_font(weather_icon, &lv_font_montserrat_20, 0);

    temp_label = lv_label_create(weather);
    lv_label_set_text(temp_label, "75°");
    lv_obj_set_style_text_color(temp_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_20, 0);
}

