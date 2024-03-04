#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <Arduino.h>

#include <esp_err.h>

#include "FS.h"
#include "SPIFFS.h"
#include "esp_log.h"
#include "types.h"
#include "config.h"
#include "xtensa/core-macros.h"
#include <string.h>

// Typedefs for clarity
typedef uint64_t delta_t;
typedef uint64_t timestamp_t;

// Define globally in one of your C files
volatile uint32_t IRAM_ATTR lastCycleCount = 0;
volatile TriggerState IRAM_ATTR lastEdgeTrigger = TriggerState::FALSE; // Use appropriate type or enum

// ISR Handler
static void IRAM_ATTR hallEffectISR(void *arg)
{
    auto gpioState = gpio_get_level(HALL_PIN);
    lastCycleCount = XTHAL_GET_CCOUNT(); // Get cycle count for timestamp

    if (gpioState == 1)
    { // Check if the interrupt was due to a rising edge
        lastEdgeTrigger = TriggerState::RISE;
    }
    else
    { // Otherwise, it's a falling edge
        lastEdgeTrigger = TriggerState::FALL;
    }
}

void writeInterrupt(uint64_t timestamp, TriggerState pinState)
{
    File file = SPIFFS.open("/data.csv", FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    file.println(String(timestamp) + "," + String(pinState - 1));
    file.close();
}

void dumpAndEraseData()
{
    File file = SPIFFS.open("/data.csv");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        Serial.println(line);
    }
    file.close();

    // Erase the data by simply removing the file
    SPIFFS.remove("/data.csv");
}

void setupInterrupt()
{
    gpio_num_t pin = static_cast<gpio_num_t>(HALL_PIN);

    // Configure the GPIO pin for the hall effect sensor
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE; // We will modify this for specific edge detection
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // Additional steps may be required to set the ISR in assembly, which might involve
    // registering the ISR with the ESP_INTR_FLAG_HIGH flag and ensuring it is in IRAM
    // Install the ISR service without an ISR handler function specified
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3));

    // Attach the ISR handler for the hall effect sensor
    ESP_ERROR_CHECK(gpio_isr_handler_add(HALL_PIN, hallEffectISR, nullptr));
}

void setup()
{
    Serial.begin(115200);
    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    //Serial.println("Dump & Erase...");
    dumpAndEraseData();
    //Serial.println("All done with D&E");
    setupInterrupt();
}

void loop()
{
    if (lastEdgeTrigger != TriggerState::FALSE)
    {
        timestamp_t microsecs = CYCLES_TO_MICROSECONDS(lastCycleCount);
        writeInterrupt(microsecs,lastEdgeTrigger);
        lastEdgeTrigger = TriggerState::FALSE;
    }
    delay(1);
}
