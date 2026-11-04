#ifndef PLUGIN_PROCESSOR_H
#define PLUGIN_PROCESSOR_H
#include "pitch_detect.hh"
#include "processor_base.hh"

class Processor : public ProcessorBase {
 public:
  Processor();
  ~Processor() override;

  // Audio processing
  virtual void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  virtual void releaseResources() override;
  virtual void processBlock(juce::AudioBuffer<float>& buffer,
                            juce::MidiBuffer& midi) override;

  // Editor
  bool hasEditor() const override { return true; }
  juce::AudioProcessorEditor* createEditor() override;

 private:
  std::unique_ptr<PitchDetect> pd;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Processor)
};
#endif  // PLUGIN_PROCESSOR_H