.section .iram1,"ax"

.literal_position
.literal GPIO_IN_REG, 0x6000403C // GPIO input register address
.literal HALL_PIN_MASK, 0x40 // Mask for GPIO2
.literal RISE, 2                  // Value for RISE
.literal FALL, 1                  // Value for FALL

.global globalCycleCountStorage
.global globalTriggerState

.text
.global hall_effect_isr
.type hall_effect_isr,@function

hall_effect_isr:
    // Read the cycle counter and store it
    rsr     a4, CCOUNT
    // Directly store the cycle count in globalCycleCountStorage
    l32r    a5, globalCycleCountStorage
    s32i    a4, a5, 0
 // Mask for the specific GPIO pin (HALL_PIN)
    movi    a5, HALL_PIN_MASK
    and     a6, a6, a5
    // Check if HALL_PIN is high or low and set triggerState accordingly
    beqz    a6, set_fall
    movi    a4, RISE
    // Directly store RISE in globalTriggerState
    l32r    a5, globalTriggerState
    s32i    a4, a5, 0
    j       isr_end

set_fall:
    movi    a4, FALL
    // Directly store FALL in globalTriggerState
    l32r    a5, globalTriggerState
    s32i    a4, a5, 0

isr_end:

    // Restore context if necessary
    retw.n
