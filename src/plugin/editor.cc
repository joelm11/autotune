#include "editor.hh"

Editor::Editor(Processor& p) : juce::AudioProcessorEditor(p) {
  pitchLabel.setText("F0: -- Hz", juce::dontSendNotification);
  pitchLabel.setJustificationType(juce::Justification::centred);
  pitchLabel.setFont(juce::FontOptions(30.0f));
  addAndMakeVisible(pitchLabel);

  setSize(400, 200);
}

void Editor::paint(juce::Graphics& g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void Editor::resized() { pitchLabel.setBounds(getLocalBounds()); }