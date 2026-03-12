#include <lvgl.h>
#include <stdio.h>
#include "music.h"
#include "../../../include/config.h"

namespace
{
    bool       s_active       = false;
    lv_obj_t*  title_label    = nullptr;
    lv_obj_t*  artist_label   = nullptr;
    lv_obj_t*  playpause_lbl  = nullptr;
    lv_obj_t*  progress_bar   = nullptr;
    lv_obj_t*  elapsed_label  = nullptr;
    lv_obj_t*  duration_label = nullptr;

    void fmt_time(char* buf, uint32_t secs)
    {
        snprintf(buf, 6, "%u:%02u", secs / 60, secs % 60);
    }

    int safe_y() { int y = (DISP_VER - 240) / 2; return y < 0 ? 0 : y; }
}

void music_create()
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
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    // Album art placeholder — teal circle with audio symbol
    lv_obj_t* art = lv_obj_create(root);
    lv_obj_set_size(art, 70, 70);
    lv_obj_align(art, LV_ALIGN_TOP_MID, 0, 38);
    lv_obj_set_style_bg_color(art, LV_COLOR_MAKE(0x20, 0xC0, 0xA0), 0);
    lv_obj_set_style_bg_opa(art, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(art, 0, 0);
    lv_obj_set_style_radius(art, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(art, 0, 0);
    lv_obj_clear_flag(art, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t* art_sym = lv_label_create(art);
    lv_label_set_text(art_sym, LV_SYMBOL_AUDIO);
    lv_obj_set_style_text_color(art_sym, lv_color_white(), 0);
    lv_obj_set_style_text_font(art_sym, &lv_font_montserrat_20, 0);
    lv_obj_center(art_sym);

    // Track title
    title_label = lv_label_create(root);
    lv_label_set_text(title_label, "No Title");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(title_label, 200);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 116);

    // Artist
    artist_label = lv_label_create(root);
    lv_label_set_text(artist_label, "Unknown");
    lv_obj_set_style_text_color(artist_label, LV_COLOR_MAKE(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(artist_label, &lv_font_montserrat_16, 0);
    lv_label_set_long_mode(artist_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(artist_label, 200);
    lv_obj_set_style_text_align(artist_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(artist_label, LV_ALIGN_TOP_MID, 0, 138);

    // Controls row — taps handled in ui.cpp TOUCH_TAP (no LVGL input device registered)
    lv_obj_t* controls = lv_obj_create(root);
    lv_obj_set_size(controls, 220, 52);
    lv_obj_align(controls, LV_ALIGN_TOP_MID, 0, 158);
    lv_obj_set_style_bg_opa(controls, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(controls, 0, 0);
    lv_obj_set_style_pad_all(controls, 0, 0);
    lv_obj_set_flex_flow(controls, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(controls, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(controls, LV_OBJ_FLAG_SCROLLABLE);

    #define MAKE_BTN(parent, symbol) ({                                        \
        lv_obj_t* _b = lv_btn_create(parent);                                 \
        lv_obj_set_size(_b, 52, 52);                                           \
        lv_obj_set_style_bg_opa(_b, LV_OPA_TRANSP, 0);                        \
        lv_obj_set_style_border_width(_b, 0, 0);                              \
        lv_obj_set_style_shadow_width(_b, 0, 0);                              \
        lv_obj_set_style_pad_all(_b, 0, 0);                                   \
        lv_obj_t* _l = lv_label_create(_b);                                   \
        lv_label_set_text(_l, symbol);                                        \
        lv_obj_set_style_text_color(_l, lv_color_white(), 0);                 \
        lv_obj_set_style_text_font(_l, &lv_font_montserrat_20, 0);            \
        lv_obj_center(_l);                                                    \
        _b;                                                                   \
    })

    MAKE_BTN(controls, LV_SYMBOL_PREV);
    lv_obj_t* pp_btn = MAKE_BTN(controls, LV_SYMBOL_PLAY);
    playpause_lbl = lv_obj_get_child(pp_btn, 0);
    MAKE_BTN(controls, LV_SYMBOL_NEXT);

    #undef MAKE_BTN

    // Progress bar
    progress_bar = lv_bar_create(root);
    lv_obj_set_size(progress_bar, 200, 4);
    lv_obj_align(progress_bar, LV_ALIGN_TOP_MID, 0, 216);
    lv_bar_set_range(progress_bar, 0, 1000);
    lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(progress_bar, LV_COLOR_MAKE(0x44, 0x44, 0x44), LV_PART_MAIN);
    lv_obj_set_style_bg_color(progress_bar, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_radius(progress_bar, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(progress_bar, 2, LV_PART_INDICATOR);

    // Time labels
    elapsed_label = lv_label_create(root);
    lv_label_set_text(elapsed_label, "0:00");
    lv_obj_set_style_text_color(elapsed_label, LV_COLOR_MAKE(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(elapsed_label, &lv_font_montserrat_14, 0);
    lv_obj_align(elapsed_label, LV_ALIGN_TOP_LEFT, 20, 222);

    duration_label = lv_label_create(root);
    lv_label_set_text(duration_label, "0:00");
    lv_obj_set_style_text_color(duration_label, LV_COLOR_MAKE(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(duration_label, &lv_font_montserrat_14, 0);
    lv_obj_align(duration_label, LV_ALIGN_TOP_RIGHT, -20, 222);
}

void music_update(const MusicState& s)
{
    if (!s_active) return;

    lv_label_set_text(title_label,  s.title);
    lv_label_set_text(artist_label, s.artist);
    lv_label_set_text(playpause_lbl, s.playing ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);

    int32_t prog = (s.duration > 0)
        ? (int32_t)((uint64_t)s.elapsed * 1000 / s.duration)
        : 0;
    lv_bar_set_value(progress_bar, prog, LV_ANIM_OFF);

    char buf[8];
    fmt_time(buf, s.elapsed);
    lv_label_set_text(elapsed_label, buf);
    fmt_time(buf, s.duration);
    lv_label_set_text(duration_label, buf);
}

void music_destroy()
{
    s_active      = false;
    title_label   = artist_label  = nullptr;
    playpause_lbl = nullptr;
    progress_bar  = elapsed_label = duration_label = nullptr;
}
