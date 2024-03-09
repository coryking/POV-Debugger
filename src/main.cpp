#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_BUS FSPI
#include <FastLED.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <Arduino.h>

#include "config.h"
#include <esp_err.h>

#include "types.h"

#include <string.h>
#include <vector>
#include "Renderers.h"
#include "RotationProfiler.h"
/// Total number of ISR calls...
volatile uint32_t IRAM_ATTR total_isr = 0;

QueueHandle_t triggerQueue;
TaskHandle_t ledRenderTask_h;

#ifdef FILEMON
RotationProfiler *_rp;
#endif

// ISR Handler
static void IRAM_ATTR hallEffectISR(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken;

    /* We have not woken a task at the start of the ISR. */
    xHigherPriorityTaskWoken = pdFALSE;

    auto gpioState = gpio_get_level(HALL_PIN);
    timestamp_t lastTriggerTime = esp_timer_get_time();
    RotationProfile data = {}; // Zero-initialize all fields
    data.isr_timestamp = lastTriggerTime;
    data.isr_trigger_number = ++total_isr;

    xQueueSendFromISR(triggerQueue, &data, &xHigherPriorityTaskWoken);
    /* Now the buffer is empty we can switch context if necessary. */
    if (xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}

// Timer handle
esp_timer_handle_t timer_handle = nullptr;

// Frame control
int currentFrame = 0;
delta_t nextCallbackMicros = 0;

FastLEDRenderer<NUM_LEDS, NUM_ARMS> *renderer;

// Function declarations
void renderFrame();

// Timer callback
void timerCallback(void *arg)
{
    renderFrame(); // Call renderFrame without parameters
}

void setTimer(delta_t delayMicros)
{
    if (timer_handle == nullptr)
    {
        const esp_timer_create_args_t timer_args = {.callback = timerCallback, .name = "frame-timer"};
        esp_timer_create(&timer_args, &timer_handle);
    }
    esp_timer_start_once(timer_handle, delayMicros);
}

void renderFrame()
{
    if (currentFrame >= numOfFrames)
    {
        currentFrame = 0; // Reset frame counter for the next rotation
        return;           // Stop the rendering chain
    }

    // Render the arms for the current frame
    renderer->renderFrame(currentFrame);

    FastLED.show();

    // Prepare for the next frame
    currentFrame++;

    // Set the timer for the next frame immediately
    setTimer(nextCallbackMicros); // This needs to be calculated beforehand
}

delta_t adjustRotationInterval(delta_t rotationInterval)
{
    return rotationInterval % 25;
}

void ledRenderTask(void *pvParameters)
{
    timestamp_t lastTimestamp = esp_timer_get_time();
    RotationProfile data;
    Serial.println("Here we are");

    while (true)
    {
        if (xQueueReceive(triggerQueue, &data, portMAX_DELAY) == pdPASS)
        {

            // Calculate the rotation interval and divide by numOfFrames for frame timing
            delta_t rotationInterval = data.isr_timestamp - lastTimestamp;
            rotationInterval = adjustRotationInterval(rotationInterval);
            nextCallbackMicros = rotationInterval / numOfFrames;
            lastTimestamp = data.isr_timestamp;

#ifdef FILEMON
            data.rotation_begin_timestamp = esp_timer_get_time();
#endif
            // Start rendering the first frame immediately, subsequent frames are timed
            renderFrame();
#ifdef FILEMON
            data.rotation_end_timestamp = esp_timer_get_time();
            _rp->logRotationProfile(data);
#endif
        }
    }
}

void setupInterrupt()
{
    gpio_num_t pin = static_cast<gpio_num_t>(HALL_PIN);

    // Configure the GPIO pin for the hall effect sensor
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE; // We will modify this for specific edge detection
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

void setupRenderTask()
{
    xTaskCreate(&ledRenderTask, "RenderTask", RTOS::LARGE_STACK_SIZE, nullptr, RTOS::HIGH_PRIORITY, &ledRenderTask_h);
}

void setup()
{
    Serial.begin(BAUD_RATE);
    delay(4000); // give us time to plug in monitoring
    triggerQueue = xQueueCreate(40, sizeof(RotationProfile));
    renderer = new DotArmRenderer<NUM_LEDS, NUM_ARMS>(numOfFrames, armMap);
    renderer->start();
#ifdef FILEMON
    Serial.println("Dump & Erase...");
    _rp = new RotationProfiler("/rp.bin");
    Serial.println("All done with D&E");

#endif
    setupRenderTask();
    Serial.println("Task created");
    setupInterrupt();
    Serial.println("Interrupt setup");
}

void loop()
{
    delay(1); // nothing to do in this loop but chillax
}
