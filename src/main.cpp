#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <Arduino.h>

#include <esp_err.h>

#include "FS.h"
#include "SPIFFS.h"
#include "config.h"
#include "esp_log.h"
#include "types.h"
#include <string.h>
#include <vector>

// Define globally in one of your C files
volatile timestamp_t IRAM_ATTR lastTriggerTime = 0;
volatile TriggerState IRAM_ATTR lastEdgeTrigger = TriggerState::FALSE; // Use appropriate type or enum

TaskHandle_t _fileTask;
QueueHandle_t _fileMon;


// ISR Handler
static void IRAM_ATTR hallEffectISR(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken;

    /* We have not woken a task at the start of the ISR. */
    xHigherPriorityTaskWoken = pdFALSE;

    auto gpioState = gpio_get_level(HALL_PIN);
    lastTriggerTime = esp_timer_get_time();
    ISRData data = {lastTriggerTime, gpioState};
    xQueueSendFromISR(_fileMon, &data, &xHigherPriorityTaskWoken);
    /* Now the buffer is empty we can switch context if necessary. */
    if (xHigherPriorityTaskWoken)
    {
        /* Actual macro used here is port specific. */
        portYIELD_FROM_ISR();
    }
}
void writeIsrEvt(File* file,ISRData isrData)
{
    file->println(String(isrData.timestamp) + "," + String(isrData.state - 1));
}
void fileMonitorTask(void *pvParameters)
{
    File file = SPIFFS.open("/data.csv", FILE_APPEND);

    while (true)
    {
        ISRData isrData;
        if (xQueueReceive(_fileMon, &isrData, portMAX_DELAY) == pdPASS)
        {
            writeIsrEvt(&file, isrData);
        }
    }
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


    delay(4000); // give us time to plug in monitoring
    Serial.println("Dump & Erase...");
    dumpAndEraseData();
    Serial.println("All done with D&E");
    _fileMon = xQueueCreate(40, sizeof(ISRData));
    xTaskCreate(&fileMonitorTask, "FileWriter", RTOS::LARGE_STACK_SIZE, nullptr, RTOS::HIGH_PRIORITY, &_fileTask);
    setupInterrupt();
}

void loop()
{
    
}
