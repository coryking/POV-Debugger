#pragma once
#include <cstdint>
#include "esp32/clk.h"

// Enum for trigger states
typedef enum : uint32_t
{
    FALSE = 0,
    FALL = 1,
    RISE = 2
} TriggerState;


#define CYCLES_TO_MICROSECONDS(cycles) ((cycles) / (esp_clk_cpu_freq() / 1000000))
