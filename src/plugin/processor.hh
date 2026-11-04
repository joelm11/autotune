#ifndef PLUGIN_PROCESSOR_H
#define PLUGIN_PROCESSOR_H
#include "editor.hh"
#include "processor_base.hh"

class Processor : public ProcessorBase {
 public:
  Processor();
  ~Processor() override;
  juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new Processor();
  }

  // Audio processing
  virtual void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  virtual void releaseResources() override;
  virtual void processBlock(juce::AudioBuffer<float>& buffer,
                            juce::MidiBuffer& midi) override;

  // Editor
  bool hasEditor() const override { return true; }
  juce::AudioProcessorEditor* createEditor() override {
    return new Editor(*this);
  }

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Processor)
};
#endif  // PLUGIN_PROCESSOR_H