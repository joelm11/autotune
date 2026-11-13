#ifndef PLUGIN_COMPONENTS_SEL_PNL_H
#define PLUGIN_COMPONENTS_SEL_PNL_H
#pragma once

#include <JuceHeader.h>

#include "../palette.hh"
#include "../state.hh"
#include "dropdown.hh"

class SelectPanel : public juce::Component {
 public:
  SelectPanel(State& state) {
    dropdown.setOptions({
        "C",
        "G",
        "D",
        "A",
        "E",
    });
    dropdown.setState(state);

    addAndMakeVisible(dropdown);
  }

  void paint(juce::Graphics& g) override { g.fillAll(palette::kMantle); }

  void resized() override {
    juce::FlexBox flex;
    flex.flexDirection = juce::FlexBox::Direction::column;
    flex.alignItems = juce::FlexBox::AlignItems::stretch;
    flex.items.add(juce::FlexItem(dropdown).withHeight(30));
    flex.performLayout(getLocalBounds());
  }

 private:
  Dropdown dropdown;
};

#endif  // PLUGIN_COMPONENTS_SEL_PNL_H