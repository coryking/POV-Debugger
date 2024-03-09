#include "RotationProfiler.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include <SPIFFS.h>

RotationProfiler::RotationProfiler(const std::string &filename) : BaseProfiler(filename, sizeof(RotationProfile))
{
}

void RotationProfiler::serializeProfile(File &file, const void *profileData)
{
    // Serialize and write to file
    uint8_t buffer[RotationProfile_size]; // Ensure buffer is large enough for your message
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (pb_encode(&stream, RotationProfile_fields, &profileData))
    {
        file.write(buffer, stream.bytes_written);
    }
    else
    {
        Serial.println("Encoding failed");
    }
}

bool RotationProfiler::deserializeProfile(File &file, void *profileData)
{
    uint8_t buffer[RotationProfile_size];
    size_t messageLength = file.readBytes((char *)buffer, sizeof(buffer));
    pb_istream_t stream = pb_istream_from_buffer(buffer, messageLength);
    return pb_decode(&stream, RotationProfile_fields, static_cast<RotationProfile *>(profileData));
}

void RotationProfiler::printProfile(const void *profileData)
{
    const RotationProfile *profile = static_cast<const RotationProfile *>(profileData);

    // Assuming profileData is already a decoded frameProfile object
    Serial.printf("%llu,%llu,%llu,%u\n", profile->isr_timestamp, profile->rotation_begin_timestamp,
                  profile->rotation_end_timestamp, profile->isr_trigger_number);
}

void RotationProfiler::printProfileHeader()
{
    Serial.println("isr_timestamp,rotation_begin_timestamp,rotation_end_timestamp,isr_trigger_number");
}

bool RotationProfiler::logRotationProfile(const RotationProfile &profile)
{
    return logProfileData(&profile);
}
