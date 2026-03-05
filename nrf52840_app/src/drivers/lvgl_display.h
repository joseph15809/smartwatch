#pragma once
#include <lvgl.h>
#include <Adafruit_SPITFT.h>

namespace lvgl_display
{
    void init(Adafruit_SPITFT& tft, int hor_res, int ver_res, int buf_lines);
    void tick_inc(uint32_t ms); // call from timer
}