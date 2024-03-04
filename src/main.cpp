#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <Arduino.h>

#include <esp_err.h>

#include "FS.h"
#include "SPIFFS.h"
#include "esp_log.h"
#include "hall_effect_isr.h"
#include <string.h>

// Typedefs for clarity
typedef uint64_t delta_t;
typedef uint64_t timestamp_t;

// Define globally in one of your C files
volatile uint32_t IRAM_ATTR globalCycleCountStorage = 0;
volatile TriggerState IRAM_ATTR globalTriggerState = TriggerState::FALSE; // Use appropriate type or enum

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
    ESP_ERROR_CHECK(gpio_isr_handler_add(HALL_PIN, hall_effect_isr, nullptr));
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
    if (globalTriggerState != TriggerState::FALSE)
    {
        timestamp_t microsecs = CYCLES_TO_MICROSECONDS(globalCycleCountStorage);
        writeInterrupt(microsecs,globalTriggerState);
        globalTriggerState = TriggerState::FALSE;
    }
    delay(1);
}
