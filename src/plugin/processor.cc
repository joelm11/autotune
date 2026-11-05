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
  fft.mag = FBuffer(samplesPerBlock);
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
  for (int i = 0; i < fft.out.size(); ++i) {
    fft.mag[i] = (std::abs(fft.out[i]));
  }
  const auto kRes = state.spectra.enqueue(fft.mag);

  // Debug
  // std::cout << "F0 detected: " << kPitch.f0 << std::endl;
}

juce::AudioProcessorEditor* Processor::createEditor() {
  return new Editor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new Processor();
}