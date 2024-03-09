#include "FrameProfiler.h"

FrameProfiler::FrameProfiler(const std::string &filename)
    : BaseProfiler(filename, "FrameProfiler", sizeof(frameProfile))
{
}

void FrameProfiler::serializeProfile(File &file, const void *profileData)
{
    const frameProfile *profile = static_cast<const frameProfile *>(profileData);

    file.printf("%d,%llu,%llu,%llu,%d\n", profile->isr_trigger_number, profile->frame_begin_timestamp,
                profile->frame_render_done_timestamp, profile->frame_end_timestamp, profile->frame_number);
}

// FrameProfiler.cpp
void FrameProfiler::printProfile(const void *profileData)
{
    const frameProfile *profile = static_cast<const frameProfile *>(profileData);

    // Assuming profileData is already a decoded frameProfile object
    Serial.printf("%d,%llu,%llu,%llu,%d\n", profile->isr_trigger_number, profile->frame_begin_timestamp,
                  profile->frame_render_done_timestamp, profile->frame_end_timestamp, profile->frame_number);
}

void FrameProfiler::printProfileHeader()
{
    Serial.println("Frame Profiler");
    Serial.println(
        "isr_trigger_number,frame_begin_timestamp,frame_render_done_timestamp,frame_end_timestamp,frame_number");
}

bool FrameProfiler::logFrameProfile(const frameProfile &profile)
{
    return logProfileData(&profile);
}
