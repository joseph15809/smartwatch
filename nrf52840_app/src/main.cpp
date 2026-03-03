#include <Arduino.h>
#include "app/app.h"


void setup() {
  Serial.begin(115200);
  delay(200);
  app:init();
}

void loop() {
  app::loop();
}
