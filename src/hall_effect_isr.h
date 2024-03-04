#pragma once

#include "esp32/clk.h"
#include "freertos/FreeRTOS.h"
#include "esp_attr.h"
#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Assembly ISR handler
    extern void hall_effect_isr(void *arg);

    // Enum for trigger states
    typedef enum : uint32_t
    {
        FALSE = 0,
        FALL = 1,
        RISE = 2
    } TriggerState;

    typedef struct __attribute__((packed))
    {
        volatile uint32_t cycle_count_storage;
        volatile TriggerState triggerState;
    } ISRData;

#define CYCLES_TO_MICROSECONDS(cycles) ((cycles) * 1000000 / esp_clk_cpu_freq())

#ifdef __cplusplus
}
#endif

