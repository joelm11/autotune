#include "PluginEditor.h"

#include "PluginProcessor.h"

//==============================================================================
HelloWorldEditor::HelloWorldEditor(HelloWorldProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
  pitchLabel.setText("F0: -- Hz", juce::dontSendNotification);
  pitchLabel.setJustificationType(juce::Justification::centred);
  pitchLabel.setFont(juce::FontOptions(30.0f));
  addAndMakeVisible(pitchLabel);

  setSize(400, 200);
  startTimerHz(20);
}

HelloWorldEditor::~HelloWorldEditor() { stopTimer(); }

//==============================================================================
void HelloWorldEditor::paint(juce::Graphics& g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void HelloWorldEditor::resized() { pitchLabel.setBounds(getLocalBounds()); }

void HelloWorldEditor::timerCallback() {
  const unsigned f0 = processorRef.getDetectedPitchHz();

  pitchLabel.setText(f0 > 0 ? "F0: " + juce::String(f0) + " Hz" : "F0: -- Hz",
                     juce::dontSendNotification);
}