#pragma once
#include "../app/events.h"

namespace ui
{
    void init();
    void tick();    // call lv time handler
    void handle(const Event& e);
}