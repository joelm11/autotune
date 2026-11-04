#include "processor.hh"

#include <memory>

#include "editor.hh"
#include "yin.hh"

Processor::Processor() : ProcessorBase() {}

Processor::~Processor() {}

void Processor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  pd = std::make_unique<Yin>(sampleRate, samplesPerBlock);
}

void Processor::releaseResources() {}

void Processor::processBlock(juce::AudioBuffer<float>& buffer,
                             juce::MidiBuffer& midi) {
  const PitchDetect::PitchType kPitch = pd->Process(
      std::vector<float>(buffer.getReadPointer(0),
                         buffer.getReadPointer(0) + buffer.getNumSamples()));

  // Debug
  std::cout << "F0 detected: " << kPitch.f0 << std::endl;
}

juce::AudioProcessorEditor* Processor::createEditor() {
  return new Editor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new Processor();
}