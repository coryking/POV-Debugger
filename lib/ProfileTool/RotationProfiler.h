#ifndef ROTATIONPROFILER_H
#define ROTATIONPROFILER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <FS.h>
#include <SPIFFS.h>
#include <string>

#include "rotationProfile.pb.h" // Include the protobuf-generated header

class RotationProfiler
{
  public:
    RotationProfiler(const std::string &filename);
    ~RotationProfiler();
    void start();
    void logRotationProfile(const RotationProfile &profile);
    void dumpToSerial(bool delWhenDone = true);

  private:
    static void fileMonitorTask(void *pvParameters);
    std::string _filename;
    TaskHandle_t _fileTaskHandle = nullptr;
    QueueHandle_t _queueHandle = nullptr;
};

#endif // ROTATIONPROFILER_H
