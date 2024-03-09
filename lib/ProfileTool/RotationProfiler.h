#ifndef ROTATIONPROFILER_H
#define ROTATIONPROFILER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <FS.h>
#include <SPIFFS.h>
#include <string>

#include "BaseProfiler.h"
typedef struct _RotationProfile
{
    uint64_t isr_timestamp;
    uint64_t rotation_begin_timestamp;
    uint64_t rotation_end_timestamp;
    uint32_t isr_trigger_number;
} RotationProfile;

class RotationProfiler : public BaseProfiler
{
  public:
    RotationProfiler(const std::string &filename);
    bool logRotationProfile(const RotationProfile &profile);

  protected:
    void serializeProfile(File &file, const void *profileData) override;
    void printProfile(const void *profileData) override;
    void printProfileHeader() override;
};
#endif // ROTATIONPROFILER_H
