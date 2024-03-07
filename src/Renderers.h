#pragma once
#include <FastLED.h>
#include <array>
template <int numOfLeds> class Renderer
{
  public:
    Renderer(int numOfFrames) : numOfFrames(numOfFrames)
    {
    }

    virtual void renderFrame(int frame, CRGB *leds)
    {
        doRenderFrame(frame, leds);
        if (frame == numOfFrames - 1)
        {
            onRotationComplete();
        }
    }

  protected:
    virtual void doRenderFrame(int frame, CRGB *leds) = 0;
    virtual void onRotationComplete()
    {
    }
    const int numOfFrames;
};

template <int numOfLeds, int numOfArms> class BaseRenderer : public Renderer<numOfLeds>
{
  public:
    BaseRenderer(int numOfFrames, const std::array<int, numOfArms> &armMap)
        : Renderer<numOfLeds>(numOfFrames), armMap(armMap), numOfLedsPerArm(numOfLeds / numOfArms)
    {
    }

  protected:
    int computeValueOffsetForArm(int value, int maxValue, int arm)
    {
        float exactOffset = (static_cast<float>(maxValue + 1) / numOfArms) * arm;
        return static_cast<int>(value + exactOffset) % (maxValue + 1);
    }
    const int numOfLedsPerArm;
    const std::array<int, numOfArms> armMap;
};

template <int numOfLeds, int numOfArms> class ArmRenderer : public BaseRenderer<numOfLeds, numOfArms>
{
  public:
    using BaseRenderer<numOfLeds, numOfArms>::BaseRenderer;

  protected:
    virtual void renderArm(int frame, int arm, CRGB *armLeds) = 0;
    void doRenderFrame(int frame, CRGB *leds)
    {
        for (int arm = 0; arm < numOfArms; arm++)
        {
            int armOffset = this->numOfLedsPerArm * armMap[arm];
            this->renderArm(frame, arm, &leds[armOffset]);
        }
    }
};

template <int numOfLeds, int numOfArms> class HueShiftRenderer : public ArmRenderer<numOfLeds, numOfArms>
{
  private:
    uint8_t hue = 0;

  public:
    using ArmRenderer<numOfLeds, numOfArms>::ArmRenderer;

  protected:
    void renderArm(int frame, int arm, CRGB *leds) override
    {
        for (int i = 0; i < numOfFrames; i++)
        {
            if (frame == 0)
            {
                int thisHue = this->computeValueOffsetForArm(hue, 255, arm);

                fill_solid(leds, this->numOfLedsPerArm, CHSV(thisHue, 255, 255));
            }
            else
            {
                fill_solid(leds, this->numOfLedsPerArm, CRGB::Black);
            }
        }
    }
    void onRotationComplete() override
    {
        ArmRenderer<numOfLeds, numOfArms>::onRotationComplete();
        hue = (hue + 1) % 256;
    }
};

// Adjusting ArmRenderer to handle AR types
template <int numOfLeds, int numOfArms, template <int> class AR>
class FancyArmRenderer : public BaseRenderer<numOfLeds, numOfArms>
{
  public:
    ArmRenderer(int numOfFrames, const std::array<int, numOfArms> &armMap)
        : BaseRenderer<numOfLeds, numOfArms>(numOfFrames, armMap)
    {
        // Initialize each arm renderer
        for (int i = 0; i < numOfArms; ++i)
        {
            armRenderers[i] = new AR<numOfLeds / numOfArms>(numOfFrames);
        }
    }

    ~ArmRenderer()
    {
        // Clean up dynamically allocated renderers
        for (int i = 0; i < numOfArms; ++i)
        {
            delete armRenderers[i];
        }
    }

  protected:
    void doRenderFrame(int frame, CRGB *leds) override
    {
        // Example implementation
    }

  private:
    AR<numOfLeds / numOfArms> *armRenderers[numOfArms];
};
template <int numOfLeds, int numOfArms> class SimpleArmRenderer : public BaseRenderer<numOfLeds, numOfArms>
{
  public:
    using BaseRenderer<numOfLeds, numOfArms>::BaseRenderer;

    void doRenderFrame(int frame, CRGB *leds) override
    {

        fill_solid(leds, numOfLeds, CRGB::Black);

        // Determine which arm and LED to illuminate for this frame
        int armToIlluminate = frame / this->numOfLedsPerArm;
        int ledPositionInArm = frame % this->numOfLedsPerArm;

        // Correct for physical layout using armMap and calculate actual position in LED array
        int actualArm = this->armMap[armToIlluminate % this->numOfArms];
        int actualPosition = actualArm * this->numOfLedsPerArm + ledPositionInArm;

        // Illuminate the calculated LED position
        if (actualPosition < numOfLeds)
        {
            leds[actualPosition] = CRGB::White;
        }
    }
};

template <int numOfLeds, int numOfArms> class LineArmRenderer : public BaseRenderer<numOfLeds, numOfArms>
{
  public:
    using BaseRenderer<numOfLeds, numOfArms>::BaseRenderer;

    void doRenderFrame(int frame, CRGB *leds) override
    {

        fill_solid(leds, numOfLeds, CRGB::Black);
        if (frame == 0)
        {
            fill_solid(leds, this->numOfLedsPerArm, CRGB::Red);
        }
    }
};