#pragma once
#include <stdint.h>

struct HealthState
{
    uint16_t heart_rate = 0;
    uint16_t distance = 0;
    uint32_t steps = 0;
};

void health_create();
void health_update(const HealthState& s);
void health_scroll(int dy);
void health_destroy();