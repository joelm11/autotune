#include "processor.hh"

#include "editor.hh"

Processor::Processor() : ProcessorBase() {}

Processor::~Processor() {}

void Processor::prepareToPlay(double sampleRate, int samplesPerBlock) {}

void Processor::releaseResources() {}

void Processor::processBlock(juce::AudioBuffer<float>& buffer,
                             juce::MidiBuffer& midi) {}

juce::AudioProcessorEditor* Processor::createEditor() {
  return new Editor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new Processor();
}