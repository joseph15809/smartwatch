#pragma once

namespace rtc { struct DateTime; }
void lockscreen_create();
void lockscreen_update(const rtc::DateTime& dt);
void lockscreen_destroy();