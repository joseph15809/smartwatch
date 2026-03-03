#pragma once
#include "events.h"

namespace app
{
    void init();
    void post(const Event& e);
    void loop();
}