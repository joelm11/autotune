#include "editor.hh"

Editor::Editor(Processor& p) : juce::AudioProcessorEditor(p) {
  addAndMakeVisible(panel_a);
  addAndMakeVisible(panel_b);

  setSize(400, 200);
}

void Editor::paint(juce::Graphics& g) {}

void Editor::resized() {
  juce::FlexBox flex;
  flex.flexDirection = FlexBox::Direction::row;
  // flex.justifyContent = FlexBox::JustifyContent::spaceBetween;
  // flex.alignItems = FlexBox::AlignItems::stretch;

  flex.items.add(FlexItem(panel_a).withFlex(3.f));
  flex.items.add(FlexItem(panel_a).withFlex(1.f));
  flex.performLayout(getLocalBounds());
}