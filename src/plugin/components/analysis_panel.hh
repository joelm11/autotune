#ifndef PLUGIN_COMPONENTS_ANL_PNL_H
#define PLUGIN_COMPONENTS_ANL_PNL_H
#include <JuceHeader.h>

#include "../state.hh"
#include "keys.hh"
#include "spectral.hh"

class AnalysisPanel : public juce::Component {
 public:
  AnalysisPanel(State& state) : spectrum(state) {
    addAndMakeVisible(spectrum);
    addAndMakeVisible(keys);
  }

  void paint(juce::Graphics&) override {}

  void resized() override {
    juce::FlexBox flex;
    flex.flexDirection = juce::FlexBox::Direction::column;
    flex.alignItems = juce::FlexBox::AlignItems::stretch;

    flex.items.add(juce::FlexItem(spectrum).withFlex(3.f));
    // flex.items.add(juce::FlexItem(keys).withFlex(1.f));
    flex.performLayout(getLocalBounds());
  }

 private:
  Spectral spectrum;
  Keys keys;
};

#endif  // PLUGIN_COMPONENTS_ANL_PNL_H