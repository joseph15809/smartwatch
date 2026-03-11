#include "config.h"
#include "lv_conf.h"
#include "app/app.h"
#include "drivers/lvgl_display.h"
#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

Adafruit_GC9A01A tft((int8_t)PIN_TFT_CS, (int8_t)PIN_TFT_DC, (int8_t)PIN_TFT_RST);
static uint32_t last_tick = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TFT_BL, OUTPUT);
  analogWrite(PIN_TFT_BL, 220);

  // init display
  tft.begin();
 tft.fillScreen(0);

  // init lvgl
  lv_init();
  lvgl_display::init(tft, DISP_HOR, DISP_VER, LV_BUF_LINES);

  // app::init() -> ui::init() -> rtc::init() + lockscreen_create()
  app::init();
  
  last_tick = millis();
}

void loop() {
  // Drive LVGL tick
  uint32_t now = millis();
  uint32_t dt = now - last_tick;
  if (dt) {
    lvgl_display::tick_inc(dt);
    last_tick = now;
  }

  app::loop();
}
