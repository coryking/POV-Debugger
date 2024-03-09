#pragma once

#include "driver/gpio.h"

#define BAUD_RATE 115200

#ifdef ARDUINO_LOLIN_S3_MINI
#define HALL_PIN GPIO_NUM_6
#else
#define HALL_PIN GPIO_NUM_2
#endif

#define FILEMON
