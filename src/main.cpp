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

/// Total number of ISR calls...
volatile uint32_t IRAM_ATTR total_isr = 0;

QueueHandle_t triggerQueue;
TaskHandle_t ledRenderTask_h;

CRGB leds[NUM_LEDS];

#ifdef FILEMON
#include <FS.h>
#include <SPIFFS.h>
TaskHandle_t _fileTask;
#endif

// ISR Handler
static void IRAM_ATTR hallEffectISR(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken;

    /* We have not woken a task at the start of the ISR. */
    xHigherPriorityTaskWoken = pdFALSE;

    auto gpioState = gpio_get_level(HALL_PIN);
    timestamp_t lastTriggerTime = esp_timer_get_time();
    ISRData data = {lastTriggerTime, gpioState, ++total_isr};
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
const int numOfFrames = 360; // Adjust as necessary
delta_t nextCallbackMicros = 0;
CRGBPalette256 myPalette = RainbowStripesColors_p;

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

void renderArmsForFrame(int frame)
{
    static const int segmap[] = {1, 0, 2}; // Correct physical mapping
    int ledsPerArm = NUM_LEDS / 3;         // LEDs per arm
    static int hue = 0;

    // Clear all LEDs
    fill_solid(&leds[0], NUM_LEDS, CRGB::Black);

    // Determine which arm and LED to illuminate for this frame
    int armToIlluminate = frame / ledsPerArm;  // Determine which arm is active based on the frame
    int ledPositionInArm = frame % ledsPerArm; // Determine the position in the active arm

    // Correct for physical layout using segmap and calculate actual position in LED array
    int actualArm = segmap[armToIlluminate % 3];                    // Ensure we loop through the arms correctly
    int actualPosition = actualArm * ledsPerArm + ledPositionInArm; // Calculate actual LED position

    // Illuminate the calculated LED position
    if (actualPosition < NUM_LEDS)
    { // Safety check
        leds[actualPosition] = CHSV(hue, 255, 255);
    }
    if (frame == numOfFrames - 1)
    {
        hue = (hue + 1) % 256;
    }
}

void renderFrame()
{
    if (currentFrame >= numOfFrames)
    {
        currentFrame = 0; // Reset frame counter for the next rotation
        return;           // Stop the rendering chain
    }

    // Render the arms for the current frame
    renderArmsForFrame(currentFrame);

    FastLED.show();

    // Prepare for the next frame
    currentFrame++;

    // Set the timer for the next frame immediately
    setTimer(nextCallbackMicros); // This needs to be calculated beforehand
}

void ledRenderTask(void *pvParameters)
{
    timestamp_t lastTimestamp = esp_timer_get_time();
    ISRData isrData;
    Serial.println("Here we are");

    while (true)
    {
        if (xQueueReceive(triggerQueue, &isrData, portMAX_DELAY) == pdPASS)
        {

            // Calculate the rotation interval and divide by numOfFrames for frame timing
            delta_t rotationInterval = isrData.timestamp - lastTimestamp;
            nextCallbackMicros = rotationInterval / numOfFrames;
            lastTimestamp = isrData.timestamp;

            // Start rendering the first frame immediately, subsequent frames are timed
            renderFrame();
        }
    }
}
#ifdef FILEMON
void writeIsrEvt(File *file, ISRData isrData)
{
    file->printf("%lu,%llu,%llu,%d\n", isrData.isrCallCount, isrData.timestamp, esp_timer_get_time(), isrData.state);
}

void fileMonitorTask(void *pvParameters)
{
    File file = SPIFFS.open("/data.csv", FILE_APPEND);
    file.println("isrCallCount,timestamp(us),writeTimestamp(us),pinState");
    ISRData isrData;
    while (true)
    {
        if (xQueueReceive(triggerQueue, &isrData, portMAX_DELAY) == pdPASS)
        {

            writeIsrEvt(&file, isrData);
        }
    }
    file.close();
}

void dumpAndEraseData()
{

    // Initialize LittleFS /

    if (!SPIFFS.begin(false))
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    File file = SPIFFS.open("/data.csv", FILE_READ);

    // File file = LittleFS.open("/data.csv");
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

#endif

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
    triggerQueue = xQueueCreate(40, sizeof(ISRData));
    FastLED.addLeds<SK9822, LED_DATA, LED_CLOCK, BGR, DATA_RATE_MHZ(LED_DATA_RATE_MHZ)>(&leds[0], NUM_LEDS);
    FastLED.setBrightness(25);

#ifdef FILEMON
    Serial.println("Dump & Erase...");
    dumpAndEraseData();
    Serial.println("All done with D&E");
    xTaskCreate(&fileMonitorTask, "FileWriter", RTOS::LARGE_STACK_SIZE, nullptr, RTOS::HIGH_PRIORITY, &_fileTask);
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
