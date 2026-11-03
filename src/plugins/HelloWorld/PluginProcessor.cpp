#include "PluginProcessor.h"

#include "PluginEditor.h"

//==============================================================================
HelloWorldProcessor::HelloWorldProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {}

//==============================================================================
void HelloWorldProcessor::prepareToPlay(double sampleRate,
                                        int samplesPerBlock) {
  // Rebuild the YIN detector for the real sample rate. Pick an analysis
  // window (~50 ms) long enough to resolve the lowest fundamental we track
  // while still matching what the host is running at.
  const int desiredFrame = static_cast<int>(sampleRate * 0.05);
  frameSize = static_cast<SizeType>(juce::jlimit(512, 8192, desiredFrame));

  yin = std::make_unique<Yin>(static_cast<unsigned>(sampleRate), frameSize);

  monoBuf.clear();
  monoBuf.reserve(frameSize);
}

void HelloWorldProcessor::releaseResources() { monoBuf.clear(); }

void HelloWorldProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& midi) {
  juce::ignoreUnused(midi);

  const int numSamples = buffer.getNumSamples();
  const int numChannels = buffer.getNumChannels();

  // Pass the audio through untouched, but feed the detector a mono mix of
  // every channel so it sees the combined signal regardless of layout.
  for (int n = 0; n < numSamples; ++n) {
    float mono = 0.f;
    for (int c = 0; c < numChannels; ++c) mono += buffer.getSample(c, n);
    mono /= static_cast<float>(numChannels);

    monoBuf.push_back(mono);
    if (monoBuf.size() >= frameSize) runPitchDetection();
  }
}

void HelloWorldProcessor::runPitchDetection() {
  if (yin == nullptr || monoBuf.size() < frameSize) return;

  const auto pitch = yin->Process(monoBuf);
  detectedPitchHz.store(pitch.f0, std::memory_order_relaxed);

  // Slide the window forward by half a frame so the estimate refreshes
  // continuously rather than in hard blocks.
  const auto half = frameSize / 2;
  monoBuf.erase(monoBuf.begin(),
                monoBuf.begin() + static_cast<std::ptrdiff_t>(half));
}

//==============================================================================
juce::AudioProcessorEditor* HelloWorldProcessor::createEditor() {
  return new HelloWorldEditor(*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new HelloWorldProcessor();
}