#include "processor.hh"

#include <memory>

#include "editor.hh"
#include "fft/fft_impl.h"
#include "yin.hh"

Processor::Processor() : ProcessorBase() {}

Processor::~Processor() {}

void Processor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  pd = std::make_unique<Yin>(sampleRate, samplesPerBlock);

  fft.out = CFBuffer(samplesPerBlock);
  fft.mag = FBuffer(samplesPerBlock / 2);
  fft.impl = std::make_unique<FFTImpl>(samplesPerBlock);
}

void Processor::releaseResources() {}

void Processor::processBlock(juce::AudioBuffer<float>& buffer,
                             juce::MidiBuffer& midi) {
  const std::vector<float> src(
      buffer.getReadPointer(0),
      buffer.getReadPointer(0) + buffer.getNumSamples());

  // Pitch
  const PitchDetect::PitchType kPitch = pd->Process(src);

  // Spectrum
  fft.impl->Forward(src, fft.out);
  fft.mag[0] = 0;
  for (int i = 1; i < fft.mag.size(); ++i) {
    fft.mag[i] = (std::abs(fft.out[i]));
  }
  state.spectra.enqueue(fft.mag);
}

juce::AudioProcessorEditor* Processor::createEditor() {
  return new Editor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new Processor();
}