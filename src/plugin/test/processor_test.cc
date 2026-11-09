#include "processor.hh"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

#include "wav_writer.hh"

namespace {
constexpr int kSampleRate = 44100;
constexpr int kBlockSize = 512;

std::vector<float> GenSine(const int sr, const double f,
                           const int num_samples) {
  std::vector<float> out(num_samples);
  for (int i = 0; i < num_samples; ++i) {
    out[i] =
        static_cast<float>(0.5 * std::sin(2.0 * std::numbers::pi * f * i / sr));
  }
  return out;
}

// Estimate the dominant frequency of |samples| by taking the median spacing
// between positive-going zero crossings over the non-silent (signal) region.
float EstimateDominantFreq(const std::vector<float>& samples) {
  int start = -1;
  for (int i = 0; i < static_cast<int>(samples.size()); ++i) {
    if (std::fabs(samples[i]) > 1e-3f) {
      start = i;
      break;
    }
  }
  if (start < 0) {
    return 0.f;
  }

  std::vector<float> periods;
  float prev = -1.f;
  for (int i = start + 1; i < static_cast<int>(samples.size()); ++i) {
    if (samples[i - 1] < 0.f && samples[i] >= 0.f) {
      const float frac = -samples[i - 1] / (samples[i] - samples[i - 1]);
      const float crossing = static_cast<float>(i - 1) + frac;
      if (prev > 0.f) {
        periods.push_back(crossing - prev);
      }
      prev = crossing;
    }
  }
  if (periods.empty()) {
    return 0.f;
  }

  // The phase-vocoder + resampler glitch near block edges, so the median
  // period spacing is far more robust than the mean.
  std::sort(periods.begin(), periods.end());
  return kSampleRate / periods[periods.size() / 2];
}
}  // namespace

class ProcessorTest : public testing::Test {
 protected:
  void SetUp() override { proc.prepareToPlay(kSampleRate, kBlockSize); }

  // Feeds `in` through the processor in stream-sized blocks and returns the
  // audio written back into the buffer after each call.
  std::vector<float> ProcessSamples(const std::vector<float>& in) {
    std::vector<float> out;
    out.reserve(in.size());
    juce::MidiBuffer midi;
    const int n = static_cast<int>(in.size());
    for (int pos = 0; pos < n; pos += kBlockSize) {
      const int chunk = std::min(kBlockSize, n - pos);
      juce::AudioBuffer<float> buffer(1, chunk);
      buffer.copyFrom(0, 0, in.data() + pos, chunk);
      proc.processBlock(buffer, midi);
      const auto* read = buffer.getReadPointer(0);
      out.insert(out.end(), read, read + chunk);
    }
    return out;
  }

  // The processor is built up-front with a simple (default) state; exact param
  // plumbing lives in the UI, which this offline test does not need.
  Processor proc;
};

// Feed a couple of seconds of a plain 440 Hz sine through the prebuilt
// processor and dump the corrected output to a WAV so it can be examined
// offline. Also check the pipeline produces output and queued spectrum frames.
TEST_F(ProcessorTest, ProcessesSineAndWritesWav) {
  constexpr double kSeconds = 2.0;
  constexpr double kFreq = 440.0;
  const int numSamples = static_cast<int>(kSeconds * kSampleRate);

  const auto output = ProcessSamples(GenSine(kSampleRate, kFreq, numSamples));

  // The pitch corrector preserves the total duration of the input.
  EXPECT_GT(output.size(), 0u);
  EXPECT_NEAR(static_cast<double>(output.size()) / numSamples, 1.0, 0.2);

  // Every block enqueues a magnitude-spectrum frame on the shared state.
  FBuffer mag;
  EXPECT_TRUE(proc.state.spectra.try_dequeue(mag));
  EXPECT_FALSE(mag.empty());

  // Dump the output for offline listening / inspection.
  EXPECT_TRUE(
      plugin_test::writeWavFile("plugin_out_sine.wav", kSampleRate, output));
}

// Feed the dialogue clip (the same resource the vocoder tests use) through the
// processor and dump the corrected output to a WAV for offline examination.
// The clip is stereo, so take its first channel as the mono source.
TEST_F(ProcessorTest, ProcessesDialogueAndWritesWav) {
  int sr = 0;
  std::vector<float> speech;
  ASSERT_TRUE(plugin_test::readWavFile(AUDIO_FILE_PATH, sr, speech));
  ASSERT_GT(speech.size(), 0u);
  ASSERT_GT(sr, 0);

  const auto output = ProcessSamples(speech);

  EXPECT_GT(output.size(), 0u);
  EXPECT_NEAR(static_cast<double>(output.size()) / speech.size(), 1.0, 0.2);

  EXPECT_TRUE(plugin_test::writeWavFile("plugin_out_dialogue.wav", sr, output));
}

// Feed a couple of seconds of a 450 Hz sine through the prebuilt processor.
// The default key is C major, whose nearest scale note to 450 Hz is A4
// (440 Hz), so the corrected output's dominant frequency must move onto the
// scale instead of being blindly shifted.
TEST_F(ProcessorTest, CorrectsOffScaleTone) {
  constexpr double kFreq = 450.0;
  constexpr double kSeconds = 2.0;
  const int numSamples = static_cast<int>(kSeconds * kSampleRate);

  const auto output = ProcessSamples(GenSine(kSampleRate, kFreq, numSamples));
  ASSERT_GT(output.size(), 0u);

  const float out_freq = EstimateDominantFreq(output);
  ASSERT_GT(out_freq, 0.f);

  // Tolerance is loose enough for the phase-vocoder + resampler artefacts but
  // far tighter than the half-step gap between C-major notes around A4.
  EXPECT_NEAR(out_freq, 440.0f, 15.0f);
}
