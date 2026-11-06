#include "processor.hh"

#include <gtest/gtest.h>

#include <cmath>
#include <numbers>
#include <vector>

#include "wav_writer.hh"

namespace {
constexpr int kSampleRate = 44100;
constexpr int kBlockSize = 512;
constexpr double kSweepStart = 200.0;
constexpr double kSweepEnd = 800.0;
}  // namespace

class ProcessorTest : public testing::Test {
 protected:
  void SetUp() override { proc.prepareToPlay(kSampleRate, kBlockSize); }

  // The processor is built up-front with a simple (default) state; exact param
  // plumbing lives in the UI, which this offline test does not need.
  Processor proc;
};

// Feed a couple of seconds of a tone sweep through the prebuilt processor and
// dump the shifted output to a WAV so it can be examined offline. Also check
// the pipeline produces output and queued spectrum frames on the shared state.
TEST_F(ProcessorTest, ProcessesSweepAndWritesWav) {
  constexpr double kSeconds = 2.0;
  const int numBlocks = static_cast<int>(kSeconds * kSampleRate) / kBlockSize;

  std::vector<float> output;
  output.reserve(static_cast<std::size_t>(numBlocks) * kBlockSize);

  juce::MidiBuffer midi;
  for (int block = 0; block < numBlocks; ++block) {
    juce::AudioBuffer<float> buffer(1, kBlockSize);
    auto* ch = buffer.getWritePointer(0);
    for (int i = 0; i < kBlockSize; ++i) {
      const int n = block * kBlockSize + i;
      const double t = static_cast<double>(n) / kSampleRate;
      const double f = kSweepStart + (kSweepEnd - kSweepStart) * t / kSeconds;
      ch[i] =
          static_cast<float>(0.5 * std::sin(2.0 * std::numbers::pi * f * t));
    }

    proc.processBlock(buffer, midi);

    const auto* read = buffer.getReadPointer(0);
    output.insert(output.end(), read, read + kBlockSize);
  }

  // The phase vocoder + resampler preserve the total duration of the input.
  EXPECT_GT(output.size(), 0u);
  EXPECT_NEAR(static_cast<double>(output.size()) / (numBlocks * kBlockSize),
              1.0, 0.2);

  // Every block enqueues a magnitude-spectrum frame on the shared state.
  FBuffer mag;
  EXPECT_TRUE(proc.state.spectra.try_dequeue(mag));
  EXPECT_FALSE(mag.empty());

  // Dump the output for offline listening / inspection.
  EXPECT_TRUE(
      plugin_test::writeWavFile("plugin_out_sweep.wav", kSampleRate, output));
}