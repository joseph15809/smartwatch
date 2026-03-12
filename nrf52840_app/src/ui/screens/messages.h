#pragma once
#include <stdint.h>

struct Conversation {
    char peer[32];     // contact name or number
    char preview[64];  // last message preview
    bool unread;
};

struct MessagesState {
    Conversation convs[8];
    uint8_t count = 0;
};

void messages_create();
void messages_update(const MessagesState& s);
void messages_scroll(int dy);
int messages_count();
int messages_scroll_y();
const char* messages_peer(int idx);
void messages_destroy();
