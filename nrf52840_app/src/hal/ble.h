#pragma once
#include "../ui/screens/notif.h"
#include "../ui/screens/messages.h"
#include "../ui/screens/music.h"
#include "../ui/screens/message_thread.h"

namespace ble
{
    void init();
    void media_prev();
    void media_playpause();
    void media_next();

    const NotifState&    notif_state();
    const MusicState&    music_state();
    const MessagesState& messages_state();
    const ThreadState&   thread_state();

    void poll(); // process ANCS queue — call from main loop
}
