#ifndef FRAMEPROFILER_H
#define FRAMEPROFILER_H

#include "BaseProfiler.h"

typedef struct _frameProfile
{
    uint32_t isr_trigger_number;
    uint64_t frame_begin_timestamp;
    uint64_t frame_render_done_timestamp;
    uint64_t frame_end_timestamp;
    uint32_t frame_number;
} frameProfile;

class FrameProfiler : public BaseProfiler
{
  public:
    FrameProfiler(const std::string &filename);
    bool logFrameProfile(const frameProfile &profile);

  protected:
    void serializeProfile(File &file, const void *profileData) override;
    void printProfile(const void *profileData) override;
    void printProfileHeader() override;
};

#endif // FRAMEPROFILER_H
