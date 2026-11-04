#ifndef PLUGIN_COMPONENTS_SEL_PNL_H
#define PLUGIN_COMPONENTS_SEL_PNL_H
#pragma once

#include <JuceHeader.h>

#include "keys.hh"
#include "spectral.hh"

class SelectPanel : public juce::Component {
 public:
  void paint(juce::Graphics& g) override {
    g.fillAll(juce::Colours::lavenderblush);
  }

  void resized() override {}

 private:
  Spectral spectrum;
  Keys keys;
};

#endif  // PLUGIN_COMPONENTS_SEL_PNL_H