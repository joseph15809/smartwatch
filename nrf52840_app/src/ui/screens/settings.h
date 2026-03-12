#pragma once
#include <stdint.h>

struct SettingsState
{
    uint8_t brightness = 80;// 0–100 %
    bool bluetooth  = false;
    bool dnd = false;
    uint8_t battery = 0;// 0–100 %
    const char* fw_ver = "v0.1.0";
};

void settings_create();
void settings_update(const SettingsState& s);
void settings_scroll(int dy);
void settings_destroy();
