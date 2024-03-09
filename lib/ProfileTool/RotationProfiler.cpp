#include "RotationProfiler.h"

#include <SPIFFS.h>

RotationProfiler::RotationProfiler(const std::string &filename)
    : BaseProfiler(filename, "RotationProfiler", sizeof(RotationProfile))
{
}

void RotationProfiler::serializeProfile(File &file, const void *profileData)
{
    const RotationProfile *profile = static_cast<const RotationProfile *>(profileData);
    file.printf("%llu,%llu,%llu,%d\n", profile->isr_timestamp, profile->rotation_begin_timestamp,
                profile->rotation_end_timestamp, profile->isr_trigger_number);
}

void RotationProfiler::printProfile(const void *profileData)
{
    const RotationProfile *profile = static_cast<const RotationProfile *>(profileData);

    // Assuming profileData is already a decoded frameProfile object
    Serial.printf("%llu,%llu,%llu,%d\n", profile->isr_timestamp, profile->rotation_begin_timestamp,
                  profile->rotation_end_timestamp, profile->isr_trigger_number);
}

void RotationProfiler::printProfileHeader()
{
    Serial.println("Rotation Profiler");
    Serial.println("isr_timestamp,rotation_begin_timestamp,rotation_end_timestamp,isr_trigger_number");
}

bool RotationProfiler::logRotationProfile(const RotationProfile &profile)
{
    return logProfileData(&profile);
}
