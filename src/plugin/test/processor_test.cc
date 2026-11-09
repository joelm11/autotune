#include "processor.hh"

#include <gtest/gtest.h>

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
// processor and dump the shifted output to a WAV so it can be examined
// offline. Also check the pipeline produces output and queued spectrum frames.
TEST_F(ProcessorTest, ProcessesSineAndWritesWav) {
  constexpr double kSeconds = 2.0;
  constexpr double kFreq = 440.0;
  const int numSamples = static_cast<int>(kSeconds * kSampleRate);

  const auto output = ProcessSamples(GenSine(kSampleRate, kFreq, numSamples));

  // The phase vocoder + resampler preserve the total duration of the input.
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
// processor and dump the shifted output to a WAV for offline examination. The
// clip is stereo, so take its first channel as the mono source.
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