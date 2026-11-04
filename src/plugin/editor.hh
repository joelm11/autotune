#ifndef PLUGIN_EDITOR_H
#define PLUGIN_EDITOR_H
#include <JuceHeader.h>

#include "processor.hh"

class Editor : public juce::AudioProcessorEditor {
 public:
  explicit Editor(Processor& p);
  ~Editor() override = default;

  void paint(juce::Graphics&) override;
  void resized() override;

 private:
  juce::Label pitchLabel;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};
#endif  // PLUGIN_EDITOR_H