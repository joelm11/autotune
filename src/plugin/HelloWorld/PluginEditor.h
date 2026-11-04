#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

//==============================================================================
// Editor that displays the fundamental frequency detected by the processor.
// Extend/override this class for real UI work.
class Editor : public juce::AudioProcessorEditor, private juce::Timer {
 public:
  explicit Editor(HelloWorldProcessor&);
  ~Editor() override;

  //==========================================================================
  void paint(juce::Graphics&) override;
  void resized() override;

 private:
  void timerCallback() override;

  HelloWorldProcessor& processorRef;
  juce::Label pitchLabel;

  //==========================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};