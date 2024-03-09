#pragma once
#include <cstdint>

// Typedefs for clarity
typedef uint64_t delta_t;
typedef uint64_t timestamp_t;

// Enum for trigger states
typedef enum : uint32_t
{
    FALSE = 0,
    FALL = 1,
    RISE = 2
} TriggerState;

struct ISRData
{
    uint64_t timestamp;
    int state;
    u32_t isrCallCount;
    timestamp_t renderStart;
    timestamp_t renderEnd;
};

namespace RTOS
{

// Priorities (adjust as needed for your system)
constexpr UBaseType_t LOW_PRIORITY = 5;
constexpr UBaseType_t NORMAL_PRIORITY = 10;
constexpr UBaseType_t HIGH_PRIORITY = 20;

// Stack sizes (adjust based on task complexity)
constexpr configSTACK_DEPTH_TYPE SMALL_STACK_SIZE = 1024 * 2;
constexpr configSTACK_DEPTH_TYPE MEDIUM_STACK_SIZE = 1024 * 3;
constexpr configSTACK_DEPTH_TYPE LARGE_STACK_SIZE = 1024 * 6;
constexpr configSTACK_DEPTH_TYPE XLARGE_STACK_SIZE = 1024 * 12;

} // namespace RTOS