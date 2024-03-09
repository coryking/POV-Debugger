#include "FrameProfiler.h"

FrameProfiler::FrameProfiler(const std::string &filename) : BaseProfiler(filename, sizeof(frameProfile))
{
}

void FrameProfiler::serializeProfile(File &file, const void *profileData)
{
    // Serialize and write to file
    uint8_t buffer[frameProfile_size]; // Ensure buffer is large enough for your message
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (pb_encode(&stream, frameProfile_fields, &profileData))
    {
        file.write(buffer, stream.bytes_written);
    }
    else
    {
        Serial.println("Encoding failed");
        // Handle error
    }
}

bool FrameProfiler::deserializeProfile(File &file, void *profileData)
{
    uint8_t buffer[frameProfile_size];
    size_t messageLength = file.readBytes((char *)buffer, sizeof(buffer));
    pb_istream_t stream = pb_istream_from_buffer(buffer, messageLength);
    return pb_decode(&stream, frameProfile_fields, static_cast<frameProfile *>(profileData));
}

// FrameProfiler.cpp
void FrameProfiler::printProfile(const void *profileData)
{
    const frameProfile *profile = static_cast<const frameProfile *>(profileData);

    // Assuming profileData is already a decoded frameProfile object
    Serial.printf("%u,%llu,%llu,%u\n", profile->isr_trigger_number, profile->frame_begin_timestamp,
                  profile->frame_end_timestamp, profile->frame_number);
}

void FrameProfiler::printProfileHeader()
{
    Serial.println("isr_trigger_number,frame_begin_timestamp,frame_end_timestamp,frame_number");
}

bool FrameProfiler::logFrameProfile(const frameProfile &profile)
{
    return logProfileData(&profile);
}
