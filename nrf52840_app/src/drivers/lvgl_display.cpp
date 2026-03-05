#include "lvgl_display.h"
#include <Arduino.h>

static Adafruit_SPITFT* s_tft = nullptr;
static lv_disp_draw_buf_t s_draw_buf;
static lv_disp_drv_t s_disp_drv;

static lv_color_t* s_buf1 = nullptr; // buffer storage

static int s_hor = 0;
static int s_ver = 0;
static int s_buf_lines = 0;

static void flush_cb(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    if(!s_tft)
    {
        lv_disp_flush_ready(disp); 
        return;
    }

    const int32_t x1 = area->x1;
    const int32_t y1 = area->y1;
    const int32_t w = (area->x2 - area->x1 + 1);
    const int32_t h = (area->y2 - area->y1 + 1);

    uint16_t* pixels = (uint16_t*)color_p;

    s_tft->startWrite();
    s_tft->setAddrWindow(x1, y1, w, h);

    s_tft->writePixels(pixels, (uint32_t)(w * h), true);
    s_tft->endWrite();

    lv_disp_flush_ready(disp); 
}

namespace lvgl_display{
    void init(Adafruit_SPITFT& tft, int hor_res, int ver_res, int buf_lines)
    {
        s_tft = &tft;
        s_hor = hor_res;
        s_ver = ver_res;
        s_buf_lines = buf_lines;

        const size_t buf_px = (size_t)s_hor * (size_t)s_buf_lines;
        s_buf1 = (lv_color_t*)malloc(buf_px * sizeof(lv_color_t));
        if (!s_buf1)
        {
            while(1) {delay(1000);}
        }

        lv_disp_draw_buf_init(&s_draw_buf, s_buf1, nullptr, (uint32_t)buf_px);
        lv_disp_drv_init(&s_disp_drv);
        s_disp_drv.hor_res = s_hor;
        s_disp_drv.ver_res = s_ver;
        s_disp_drv.flush_cb = flush_cb;
        s_disp_drv.draw_buf = &s_draw_buf;

        lv_disp_drv_register(&s_disp_drv);
    }

    void tick_inc(uint32_t ms)
    {
        lv_tick_inc(ms);
    }
} // namespace lvgl_display