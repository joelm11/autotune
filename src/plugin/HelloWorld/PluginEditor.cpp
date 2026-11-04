#include "PluginEditor.h"

#include "PluginProcessor.h"

//==============================================================================
Editor::Editor(HelloWorldProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
  pitchLabel.setText("F0: -- Hz", juce::dontSendNotification);
  pitchLabel.setJustificationType(juce::Justification::centred);
  pitchLabel.setFont(juce::FontOptions(30.0f));
  addAndMakeVisible(pitchLabel);

  setSize(400, 200);
  startTimerHz(20);
}

Editor::~Editor() { stopTimer(); }

//==============================================================================
void Editor::paint(juce::Graphics& g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void Editor::resized() { pitchLabel.setBounds(getLocalBounds()); }

void Editor::timerCallback() {
  const unsigned f0 = processorRef.getDetectedPitchHz();

  pitchLabel.setText(f0 > 0 ? "F0: " + juce::String(f0) + " Hz" : "F0: -- Hz",
                     juce::dontSendNotification);
}