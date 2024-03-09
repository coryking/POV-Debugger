#include "BaseProfiler.h"

BaseProfiler::BaseProfiler(const std::string &filename, const std::string &taskname, size_t profileSize)
    : _filename(filename), _taskname(taskname), _profileSize(profileSize)
{
    _queueHandle = xQueueCreate(10, profileSize); // Adjust the queue size as necessary
}

BaseProfiler::~BaseProfiler()
{
    if (_queueHandle != nullptr)
    {
        vQueueDelete(_queueHandle);
    }
}

void BaseProfiler::start()
{
    xTaskCreate(&fileMonitorTask, _taskname.c_str(), 1024 * 6, this, 10, &this->_fileTaskHandle);
}

bool BaseProfiler::logProfileData(const void *profileData)
{
    if (xQueueSend(_queueHandle, profileData, portMAX_DELAY) != pdPASS)
    {
        Serial.println("Failed to enqueue profile data");
        return false;
    }
    return true;
}

void BaseProfiler::fileMonitorTask(void *pvParameters)
{
    auto *profiler = static_cast<BaseProfiler *>(pvParameters); // Cast the void pointer back to BaseProfiler pointer

    File file = SPIFFS.open(profiler->_filename.c_str(), FILE_WRITE); // Open the file for writing
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        vTaskDelete(nullptr); // Delete the task if file opening fails
        return;
    }

    void *profileData = malloc(profiler->_profileSize); // Allocate memory for profile data
    if (!profileData)
    {
        Serial.println("Memory allocation failed for profile data");
        file.close();
        vTaskDelete(nullptr);
        return;
    }

    while (true)
    {
        if (xQueueReceive(profiler->_queueHandle, profileData, portMAX_DELAY) == pdPASS)
        {
            // Call the subclass-specific serializeProfile method to handle serialization
            profiler->serializeProfile(file, profileData);
        }
        // Additional task logic (e.g., checking for task termination conditions) goes here
    }

    free(profileData);    // Clean up allocated memory
    file.close();         // Ensure the file is closed before task deletion
    vTaskDelete(nullptr); // Delete this task if we ever break out of the loop
}

void BaseProfiler::dumpToSerial(bool delWhenDone)
{
    File file = SPIFFS.open(_filename.c_str(), FILE_READ); // Open the file for reading

    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    // Call the subclass-specific method to print the CSV header
    this->printProfileHeader();

    // Temporary buffer and profile data object allocation could be done here
    // For illustration, we'll assume a generic buffer size and that deserializeProfile manages the details
    void *profileData = malloc(this->_profileSize); // Allocate memory for profile data based on subclass-specific size
    if (!profileData)
    {
        Serial.println("Failed to allocate memory for profile data");
        file.close();
        return;
    }

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        Serial.println(line);
    }

    file.close(); // Close the file when done

    // Delete the file if requested
    if (delWhenDone)
    {
        SPIFFS.remove(_filename.c_str());
    }

    // Free the allocated memory for profile data
    free(profileData);
}
