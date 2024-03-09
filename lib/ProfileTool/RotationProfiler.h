#ifndef ROTATIONPROFILER_H
#define ROTATIONPROFILER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <FS.h>
#include <SPIFFS.h>
#include <string>

#include "BaseProfiler.h"
#include "rotationProfile.pb.h" // Include the protobuf-generated header

class RotationProfiler : public BaseProfiler
{
  public:
    RotationProfiler(const std::string &filename);
    bool logRotationProfile(const RotationProfile &profile);

  protected:
    void serializeProfile(File &file, const void *profileData) override;
    bool deserializeProfile(File &file, void *profileData) override;
    void printProfile(const void *profileData) override;
    void printProfileHeader() override;
};
#endif // ROTATIONPROFILER_H
