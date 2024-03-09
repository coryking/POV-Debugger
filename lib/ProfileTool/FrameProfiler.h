#ifndef FRAMEPROFILER_H
#define FRAMEPROFILER_H

#include "BaseProfiler.h"
#include "frameProfile.pb.h"

class FrameProfiler : public BaseProfiler
{
  public:
    FrameProfiler(const std::string &filename);
    bool logFrameProfile(const frameProfile &profile);

  protected:
    void serializeProfile(File &file, const void *profileData) override;
    bool deserializeProfile(File &file, void *profileData) override;
    void printProfile(const void *profileData) override;
    void printProfileHeader() override;
};

#endif // FRAMEPROFILER_H
