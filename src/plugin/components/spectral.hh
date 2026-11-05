#ifndef PLUGIN_COMPONENTS_SPECTRAL_H
#define PLUGIN_COMPONENTS_SPECTRAL_H
#pragma once
#include <JuceHeader.h>

class Spectral : public juce::Component {
 public:
  void paint(juce::Graphics& g) override {
    // Paint the cached spectrum

    g.fillAll(juce::Colours::orange);
  }

  void resized() override {}

  void update() {
    // Read the newest spectrum
    //
  }

 private:
};
#endif  // PLUGIN_COMPONENTS_SPECTRAL_H