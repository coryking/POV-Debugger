#pragma once
#define LED_CLOCK 7
#define LED_DATA 9

#include "driver/gpio.h"

#define LED_DATA_RATE_MHZ 40

#define BAUD_RATE 115200

#define NUM_LEDS 30

#ifdef ARDUINO_LOLIN_S3_MINI
#define HALL_PIN GPIO_NUM_6
#else
#define HALL_PIN GPIO_NUM_2
#endif

// #define FILEMON
