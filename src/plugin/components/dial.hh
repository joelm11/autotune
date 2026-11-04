#ifndef PLUGIN_COMPONENTS_DIAL_H
#define PLUGIN_COMPONENTS_DIAL_H

#include <JuceHeader.h>

class Dial : public juce::Slider {
 public:
  Dial()
      : Slider(juce::Slider::SliderStyle::RotaryVerticalDrag,
               juce::Slider::TextBoxBelow) {
    setRange(0.0, 1.0, 0.0);
    setValue(0.5);
    setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    setColour(juce::Slider::thumbColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxBackgroundColourId,
              juce::Colour(0xff202020));
  }

  //   void paint(juce::Graphics&) override {}
  //   void resized() override {}

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Dial)
};

#endif  // PLUGIN_COMPONENTS_DIAL_H