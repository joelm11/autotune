#ifndef PLUGIN_EDITOR_H
#define PLUGIN_EDITOR_H
#include <JuceHeader.h>

#include "components/analysis_panel.hh"
#include "components/select_panel.hh"
#include "processor.hh"

class Editor : public juce::AudioProcessorEditor {
 public:
  explicit Editor(Processor& p);
  ~Editor() override = default;

  void paint(juce::Graphics&) override;
  void resized() override;

 private:
  AnalysisPanel panel_a;
  SelectPanel panel_b;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};
#endif  // PLUGIN_EDITOR_H