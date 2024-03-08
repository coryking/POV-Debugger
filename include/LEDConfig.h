#pragma once
#include <FastLED.h>
#include <array>

#define LED_CLOCK 7
#define LED_DATA 9
#define LED_DATA_RATE_MHZ 40
#define NUM_LEDS 30
#define NUM_ARMS 3

const std::array<int, NUM_ARMS> armMap = {1, 0, 2};
const int numOfFrames = 9; // Adjust as necessary

class LEDConfigurator
{
  public:
    static void setupFastLED(CRGB *leds)
    {
        FastLED.addLeds<SK9822, LED_DATA, LED_CLOCK, BGR, DATA_RATE_MHZ(LED_DATA_RATE_MHZ)>(leds, NUM_LEDS);
        FastLED.setBrightness(25);
    }
};
