#include "editor.hh"

#include "components/analysis_panel.hh"

Editor::Editor(Processor& p)
    : juce::AudioProcessorEditor(p), panel_a(p.state), panel_b(p.state) {
  // Install the mono LookAndFeel as the default so fonts created directly
  // (e.g. Graphics::setFont) resolve to the monospaced typeface too; binding
  // it to the editor also covers all child widgets.
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