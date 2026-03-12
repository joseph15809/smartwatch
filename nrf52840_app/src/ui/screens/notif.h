#pragma once
#include <stdint.h>
#include "homescreen.h"   // AppId

struct NotifEntry {
    AppId source;
    char  title[32];
    char  body[64];
};

struct NotifState {
    NotifEntry entries[8];
    uint8_t    count = 0;
};

void  notif_create();
void  notif_update(const NotifState& s);
void  notif_scroll(int dy);
void  notif_dismiss(int idx);
void  notif_clear_all();
int   notif_count();
AppId notif_source(int idx);
int   notif_scroll_y();
void  notif_destroy();
