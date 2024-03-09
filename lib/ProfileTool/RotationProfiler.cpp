#include "RotationProfiler.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include <SPIFFS.h>

RotationProfiler::RotationProfiler(const std::string &filename) : _filename(filename)
{
    _queueHandle = xQueueCreate(10, sizeof(RotationProfile)); // Adjust size as needed
    if (_queueHandle == nullptr)
    {
        Serial.println("Failed to create queue");
        // Handle error
    }
}

RotationProfiler::~RotationProfiler()
{
    if (_queueHandle != nullptr)
    {
        vQueueDelete(_queueHandle);
    }
}

void RotationProfiler::start()
{
    xTaskCreate(fileMonitorTask, "FileMonitorTask", 2048, this, 1, &_fileTaskHandle);
}

void RotationProfiler::logRotationProfile(const RotationProfile &profile)
{
    if (xQueueSend(_queueHandle, &profile, portMAX_DELAY) != pdPASS)
    {
        Serial.println("Failed to send profile to queue");
        // Handle error
    }
}

void RotationProfiler::fileMonitorTask(void *pvParameters)
{
    auto *instance = static_cast<RotationProfiler *>(pvParameters);
    SPIFFS.begin(true);
    File file = SPIFFS.open(instance->_filename.c_str(), FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        vTaskDelete(nullptr); // End task
    }

    RotationProfile profile;
    while (true)
    {
        if (xQueueReceive(instance->_queueHandle, &profile, portMAX_DELAY) == pdPASS)
        {
            // Serialize and write to file
            uint8_t buffer[RotationProfile_size]; // Ensure buffer is large enough for your message
            pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
            if (pb_encode(&stream, RotationProfile_fields, &profile))
            {
                file.write(buffer, stream.bytes_written);
            }
            else
            {
                Serial.println("Encoding failed");
                // Handle error
            }
        }
    }
    file.close();
    vTaskDelete(nullptr); // End task if loop exits
}

void RotationProfiler::dumpToSerial(bool delWhenDone)
{
    // Implementation of dumping file contents to serial
    // This is an exercise left to the reader
}
