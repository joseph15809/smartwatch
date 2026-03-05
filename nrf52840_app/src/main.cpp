#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "config.h"
#include "app/app.h"
#include "drivers/lvgl_display.h"

Adafruit_ST7789 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
static uint32_t last_tick = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TFT_BL, OUTPUT);
  analogWrite(PIN_TFT_BL, 220);

  // init display
  tft.init(DISP_HOR, DISP_VER);
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);

  // init lvgl
  lv_init();
  lvgl_display::init(tft, DISP_HOR, DISP_VER, LV_BUF_LINES);

  // Create a quick test UI
  lv_obj_t* label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "LVGL on nRF52840");
  lv_obj_center(label);

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

  lv_timer_handler(); // run LVGL
  delay(5);           // keep CPU chill; can remove later
}
