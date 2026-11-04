#ifndef PLUGIN_PROCESSOR_BASE_H
#define PLUGIN_PROCESSOR_BASE_H
#include <JuceHeader.h>

class ProcessorBase : public juce::AudioProcessor {
 public:
  ProcessorBase()
      : AudioProcessor(
            BusesProperties()
                .withInput("Input", juce::AudioChannelSet::mono(), true)
                .withOutput("Output", juce::AudioChannelSet::mono(), true)) {}
  ~ProcessorBase() override = default;

  // Audio processing
  virtual void prepareToPlay(double sampleRate,
                             int samplesPerBlock) override = 0;
  virtual void releaseResources() override = 0;
  virtual void processBlock(juce::AudioBuffer<float>& buffer,
                            juce::MidiBuffer& midi) override = 0;

  // Editor
  virtual juce::AudioProcessorEditor* createEditor() override = 0;
  bool hasEditor() const override { return true; }

  // Global plugin configs
  const juce::String getName() const override { return JucePlugin_Name; }
  double getTailLengthSeconds() const override { return 0.0; }
  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }
  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int index) override {}
  const juce::String getProgramName(int index) override { return {}; }
  void changeProgramName(int index, const juce::String& newName) override {}
  void getStateInformation(juce::MemoryBlock& destData) override {}
  void setStateInformation(const void* data, int sizeInBytes) override {}

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorBase)
};
#endif  // PLUGIN_PROCESSOR_BASE_H