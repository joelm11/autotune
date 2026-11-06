#ifndef PLUGIN_COMPONENTS_SEL_PNL_H
#define PLUGIN_COMPONENTS_SEL_PNL_H
#pragma once

#include <JuceHeader.h>

#include "../state.hh"

class SelectPanel : public juce::Component {
 public:
  SelectPanel(State& state) {
    label.setText("Defaults to C4",
                  juce::NotificationType::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::aliceblue);
    label.setColour(juce::Label::backgroundColourId, juce::Colours::black);
    label.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    label.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(label);
  }

  void paint(juce::Graphics& g) override {
    g.fillAll(juce::Colours::lavenderblush);
  }

  void resized() override {
    juce::FlexBox flex;
    flex.flexDirection = FlexBox::Direction::column;
    flex.alignItems = FlexBox::AlignItems::stretch;
    flex.items.add(FlexItem(label).withHeight(30));
    flex.performLayout(getLocalBounds());
  }

 private:
  juce::Label label;
};

#endif  // PLUGIN_COMPONENTS_SEL_PNL_H