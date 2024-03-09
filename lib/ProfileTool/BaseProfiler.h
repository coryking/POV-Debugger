#ifndef BASEPROFILER_H
#define BASEPROFILER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <FS.h>
#include <SPIFFS.h>
#include <string>

#include "pb_encode.h"
#include "pb_decode.h"

class BaseProfiler
{
  public:
    BaseProfiler(const std::string &filename, size_t profileSize);
    virtual ~BaseProfiler();
    void start();
    void dumpToSerial(bool delWhenDone = true);

  protected:
    bool logProfileData(const void *profileData);

    virtual void serializeProfile(File &file, const void *profileData) = 0;
    virtual bool deserializeProfile(File &file, void *profileData) = 0;

    virtual void printProfile(const void *profileData) = 0;
    virtual void printProfileHeader() = 0;

    static void fileMonitorTask(void *pvParameters);

    std::string _filename;
    TaskHandle_t _fileTaskHandle = nullptr;
    QueueHandle_t _queueHandle = nullptr;
    size_t _profileSize;
};

#endif // BASEPROFILER_H
