#ifndef PLUGIN_COMPONENTS_KEYS_H
#define PLUGIN_COMPONENTS_KEYS_H
#pragma once
#include <JuceHeader.h>

class Keys : public juce::Component {
 public:
  void paint(juce::Graphics& g) override {
    g.fillAll(juce::Colours::yellowgreen);
  }

  void resized() override {}

 private:
};

#endif  // PLUGIN_COMPONENTS_KEYS_H