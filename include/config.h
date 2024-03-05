#ifndef __CONFIG_H__
#define __CONFIG_H__
#include "driver/gpio.h"

#ifdef ARDUINO_LOLIN_S3_MINI
#define HALL_PIN GPIO_NUM_6
#else
#define HALL_PIN GPIO_NUM_2
#endif
// (1<<GPIO_NUM_6)
#define HALL_PIN_MASK 0x40
#endif // __CONFIG_H__