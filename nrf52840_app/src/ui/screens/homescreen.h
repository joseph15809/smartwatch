#pragma once
#include <stdint.h>

enum AppId : uint8_t
{
    HEALTH = 0,
    MUSIC,
    MESSAGES,
    SETTINGS,
    COUNT
};

void homescreen_create();
AppId homescreen_selected_at(int idx);
void homescreen_destroy();