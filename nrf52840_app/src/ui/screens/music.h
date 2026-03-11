#pragma once
#include <stdint.h>

struct MusicState
{
    const char* title = "No Title";
    const char* artist = "Unknown";
    uint32_t elapsed = 0;
    uint32_t duration = 180; // 3min
    bool playing = false;
};

void music_create();
void music_update(const MusicState& s);
void music_destroy();