#pragma once
#include <FastLED.h>
#include "LEDConfig.h"
#include <array>
template <int numOfLeds> class Renderer
{
  public:
    Renderer(int numOfFrames) : numOfFrames(numOfFrames)
    {
    }
    void start()
    {
        fill_solid(this->_leds, numOfLeds, CRGB::Black);
        this->onStart(this->_leds);
        this->_started = 1;
    }
    void renderFrame(int frame)
    {
        assert(this->_started == 1);
        if (frame == 0)
        {
            this->onRotationBegin(this->_leds);
        }

        this->onRenderFrame(frame, this->_leds);

        if (frame == numOfFrames - 1)
        {
            this->onRotationComplete(this->_leds);
        }
    }

  protected:
    virtual void onRenderFrame(int frame, CRGB *leds) = 0;
    virtual void onRotationBegin(CRGB *leds)
    {
    }
    virtual void onRotationComplete(CRGB *leds)
    {
    }
    virtual void onStart(CRGB *leds)
    {
    }
    const int numOfFrames;

  private:
    CRGB _leds[numOfLeds];
    bool _started = 0;
};

template <int numOfLeds, int numOfArms> class FastLEDRenderer : public Renderer<numOfLeds>
{
  public:
    using Base = Renderer<numOfLeds>;
    FastLEDRenderer(int numOfFrames, const std::array<int, numOfArms> &armMap)
        : Base(numOfFrames), armMap(armMap), numOfLedsPerArm(numOfLeds / numOfArms)
    {
    }

  protected:
    void onStart(CRGB *leds) override
    {
        Base::onStart(leds);
        LEDConfigurator::setupFastLED(leds);
    }
    int computeValueOffsetForArm(int value, int maxValue, int arm)
    {
        float exactOffset = (static_cast<float>(maxValue + 1) / numOfArms) * arm;
        return static_cast<int>(value + exactOffset) % (maxValue + 1);
    }
    const int numOfLedsPerArm;
    const std::array<int, numOfArms> armMap;
};

template <int numOfLeds, int numOfArms> class ArmRenderer : public FastLEDRenderer<numOfLeds, numOfArms>
{
  public:
    using FastLEDRenderer<numOfLeds, numOfArms>::FastLEDRenderer;

  protected:
    virtual void renderArm(int frame, int arm, CRGB *armLeds) = 0;
    void onRenderFrame(int frame, CRGB *leds)
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

  public:
    using AR = ArmRenderer<numOfLeds, numOfArms>;
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
    void onRotationComplete(CRGB *leds) override
    {
        AR::onRotationComplete(leds);
        hue = (hue + 1) % 256;
    }

  private:
    uint8_t hue = 0;
};
/*
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
    void onRenderFrame(int frame, CRGB *leds) override
    {
        // Example implementation
    }

  private:
    AR<numOfLeds / numOfArms> *armRenderers[numOfArms];
};
*/
template <int numOfLeds, int numOfArms> class DotArmRenderer : public FastLEDRenderer<numOfLeds, numOfArms>
{
  public:
    using FastLEDRenderer<numOfLeds, numOfArms>::FastLEDRenderer;

    void onRenderFrame(int frame, CRGB *leds) override
    {
        if (frame == 0)
        {
            leds[0] = CRGB::White;
        }
        else
        {
            leds[0] = CRGB::Black;
        }
    }
};

template <int numOfLeds, int numOfArms> class LineArmRenderer : public FastLEDRenderer<numOfLeds, numOfArms>
{
  public:
    using FastLEDRenderer<numOfLeds, numOfArms>::FastLEDRenderer;

    void onRenderFrame(int frame, CRGB *leds) override
    {

        fill_solid(leds, numOfLeds, CRGB::Black);
        if (frame == 0)
        {
            fill_solid(leds, this->numOfLedsPerArm, CRGB::Red);
        }
    }
};