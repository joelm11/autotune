#include "processor.hh"

#include <memory>

#include "editor.hh"
#include "fft/fft_impl.h"
#include "pitch_correct.hh"
#include "yin.hh"

Processor::Processor() : ProcessorBase() {}

Processor::~Processor() {}

void Processor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  pd = std::make_unique<Yin>(sampleRate, samplesPerBlock);
  // Default to full correction onto the C-major scale; the note targets are
  // derived from the detected fundamental, so no block size is needed here.
  pc = std::make_unique<PitchCorrect>(pitch::Key::C, 1.f);

  fft.out = CFBuffer(samplesPerBlock);
  fft.mag = FBuffer(samplesPerBlock / 2);
  fft.impl = std::make_unique<FFTImpl>(samplesPerBlock);
}

void Processor::releaseResources() {}

void Processor::processBlock(juce::AudioBuffer<float>& buffer,
                             juce::MidiBuffer& midi) {
  pc->SetKey(state.key.load());

  const std::vector<float> src(
      buffer.getReadPointer(0),
      buffer.getReadPointer(0) + buffer.getNumSamples());

  // Pitch detect
  const PitchDetect::PitchType kPitch = pd->Process(src);

  // Pitch correction
  const auto kCorrected = pc->Process(src, static_cast<float>(kPitch.f0));

  // Write corrected audio back to buffer
  if (!kCorrected.empty()) {
    buffer.copyFrom(0, 0, kCorrected.data(), kCorrected.size());
  }

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