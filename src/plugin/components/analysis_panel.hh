#ifndef PLUGIN_COMPONENTS_ANL_PNL_H
#define PLUGIN_COMPONENTS_ANL_PNL_H
#pragma once

#include <JuceHeader.h>

#include "keys.hh"
#include "spectral.hh"

class AnalysisPanel : public juce::Component {
 public:
  AnalysisPanel() {
    addAndMakeVisible(spectrum);
    addAndMakeVisible(keys);
  }

  void paint(juce::Graphics& g) override {
    juce::FlexBox flex;
    flex.flexDirection = FlexBox::Direction::column;
    flex.alignItems = FlexBox::AlignItems::stretch;

    flex.items.add(FlexItem(spectrum).withFlex(3.f));
    flex.items.add(FlexItem(keys).withFlex(1.f));
    flex.performLayout(getLocalBounds());
  }

  void resized() override {}

 private:
  Spectral spectrum;
  Keys keys;
};

#endif  // PLUGIN_COMPONENTS_ANL_PNL_H