#pragma once
#include <stdint.h>

struct ThreadMessage {
    char text[128];
    bool sent; // true = sent by me, false = received
};

struct ThreadState {
    char peer[32];
    ThreadMessage messages[32];
    uint8_t count = 0;
};

void thread_create(const ThreadState& s);
void thread_scroll(int dy);
void thread_destroy();
