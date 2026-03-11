#include <lvgl.h>
#include <stdio.h> 
#include "lockscreen.h"
#include "../../../include/lv_conf.h"
#include "../../../include/config.h"
#include "../../hal/rtc.h"  

namespace
{
    bool s_active = false; // true while lockscreen owns the screen
    lv_obj_t*  hour_label  = nullptr;
    lv_obj_t*  min_label   = nullptr;
    lv_obj_t*  date1_label = nullptr;
    lv_obj_t*  date2_label = nullptr;
    lv_obj_t*  temp_label  = nullptr;
    lv_obj_t*  weather_icon = nullptr;

    const char* kWday[7] = { "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

    const char* kMonth[13] = { "","January","February","March","April","May","June", "July","August","September","October","November","December"};

    static int safe_y() {
        // Dev screen is 240x280
        // Round screen later is 240x240
        int y = (DISP_VER - 240) / 2;
        return (y < 0) ? 0 : y;
    }
}

void lockscreen_create()
{
    s_active = true;
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t* root = lv_obj_create(scr);
    lv_obj_set_size(root, 240, 240);
    lv_obj_set_pos(root, 0, safe_y());
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);

    // root: two-column flex row
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(root, 10, 0);

    //left column: hour / min
    lv_obj_t* left = lv_obj_create(root);
    lv_obj_set_size(left, 105, 240);
    lv_obj_set_style_bg_opa(left, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left, 0, 0);
    lv_obj_set_style_pad_left(left, 34, 0);
    lv_obj_set_style_pad_top(left, 55, 0);
    lv_obj_set_style_pad_row(left, 18, 0);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(left, LV_OBJ_FLAG_SCROLLABLE);

    hour_label = lv_label_create(left);
    lv_label_set_text(hour_label, "--");
    lv_obj_set_style_text_color(hour_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(hour_label, &lv_font_montserrat_48, 0);

    min_label = lv_label_create(left);
    lv_label_set_text(min_label, "--");
    lv_obj_set_style_text_color(min_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(min_label, &lv_font_montserrat_48, 0);

    // right column: date + weather
    lv_obj_t* right = lv_obj_create(root);
    lv_obj_set_size(right, 140, 240);
    lv_obj_set_style_bg_opa(right, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right, 0, 0);
    lv_obj_set_style_pad_top(right, 76, 0);
    lv_obj_set_style_pad_row(right, 10, 0);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(right, LV_OBJ_FLAG_SCROLLABLE);

    date1_label = lv_label_create(right);
    lv_label_set_text(date1_label, "-- -----");
    lv_obj_set_style_text_color(date1_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(date1_label, &lv_font_montserrat_16, 0);

    date2_label = lv_label_create(right);
    lv_label_set_text(date2_label, "------");
    lv_obj_set_style_text_color(date2_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(date2_label, &lv_font_montserrat_16, 0);

    // Weather row (icon + temp)
    lv_obj_t* weather = lv_obj_create(right);
    lv_obj_set_size(weather, 90, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(weather, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(weather, 0, 0);
    lv_obj_set_style_pad_all(weather, 0, 0);
    lv_obj_set_flex_flow(weather, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(weather, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(weather, 10, 0);

    weather_icon = lv_label_create(weather);
    lv_label_set_text(weather_icon, LV_SYMBOL_UP);
    lv_obj_set_style_text_color(weather_icon, lv_color_white(), 0);
    lv_obj_set_style_text_font(weather_icon, &lv_font_montserrat_20, 0);

    temp_label = lv_label_create(weather);
    lv_label_set_text(temp_label, "--°");
    lv_obj_set_style_text_color(temp_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_20, 0);
}

// lockscreen_update — mutates labels only
void lockscreen_update(const rtc::DateTime& dt)
{
    if(!s_active) return;
    char buf[16];

    snprintf(buf, sizeof(buf), "%02d", dt.hour);
    lv_label_set_text(hour_label, buf);

    snprintf(buf, sizeof(buf), "%02d", dt.min);
    lv_label_set_text(min_label, buf);

    const char* month_str = (dt.month >= 1 && dt.month <= 12) ? kMonth[dt.month] : "?";
    snprintf(buf, sizeof(buf), "%d, %s", dt.day, month_str);
    lv_label_set_text(date1_label, buf);

    const char* wday_str = (dt.day <= 6) ? kWday[dt.wday] : "?";
    lv_label_set_text(date2_label, wday_str);

    // weather later added with BLE
}

// prevents lockscreen_update() from firing into freed objects during the
// brief window between show() and the next TICK_1S.
void lockscreen_destroy()
{
    s_active = false;
    hour_label = min_label = date1_label = date2_label = nullptr;
    temp_label = weather_icon = nullptr;
}

