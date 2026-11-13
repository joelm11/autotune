#include "editor.hh"

#include "components/analysis_panel.hh"
#include "components/look_and_feel.hh"

Editor::Editor(Processor& p)
    : juce::AudioProcessorEditor(p), panel_a(p.state), panel_b(p.state) {
  juce::LookAndFeel::setDefaultLookAndFeel(&monoLookAndFeel());
  setLookAndFeel(&monoLookAndFeel());

  addAndMakeVisible(panel_a);
  addAndMakeVisible(panel_b);

  setSize(450, 250);
}

Editor::~Editor() { setLookAndFeel(nullptr); }

void Editor::paint(juce::Graphics& g) {}
void Editor::resized() {
  juce::FlexBox flex;
  flex.flexDirection = juce::FlexBox::Direction::row;
  flex.alignItems = juce::FlexBox::AlignItems::stretch;

  flex.items.add(juce::FlexItem(panel_a).withFlex(3.f));
  flex.items.add(juce::FlexItem(panel_b).withFlex(1.f));
  flex.performLayout(getLocalBounds());
}