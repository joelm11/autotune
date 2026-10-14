#include "../ola_buffer.h"

#include <gtest/gtest.h>

#include "AudioFile.h"

TEST(OLABufferTest, OverlapAdd0) {
  const size_t window_size = 4;
  const size_t hop_size = 0;

  // Shouldn't be able to construct an OLARBuffer with hop_size = 0.
}

TEST(OLABufferTest, OverlapAdd1) {
  const size_t window_size = 4;
  const size_t hop_size = 2;
  const std::vector<float> samples = {1.0f, 1.0f, 1.0f, 1.0f};
  OLABuffer<float> buffer(window_size, hop_size);

  EXPECT_EQ(buffer.OverlapAdd(samples), std::vector<float>({1.0f, 1.0f}));
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(buffer.OverlapAdd(samples), std::vector<float>({2.0f, 2.0f}));
  }
}

TEST(OLABufferTest, OverlapAdd2) {
  const size_t window_size = 4;
  const size_t hop_size = 1;
  const std::vector<float> samples = {1.0f, 1.0f, 1.0f, 1.0f};
  OLABuffer<float> buffer(window_size, hop_size);

  EXPECT_EQ(buffer.OverlapAdd(samples), std::vector<float>({1.0f}));
  EXPECT_EQ(buffer.OverlapAdd(samples), std::vector<float>({2.0f}));
  EXPECT_EQ(buffer.OverlapAdd(samples), std::vector<float>({3.0f}));
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(buffer.OverlapAdd(samples), std::vector<float>({4.0f}));
  }
}

TEST(OLABufferTest, OverlapAdd3) {
  const size_t window_size = 4;
  const size_t hop_size = 3;
  const std::vector<float> samples = {1.0f, 1.0f, 1.0f, 1.0f};
  OLABuffer<float> buffer(window_size, hop_size);

  EXPECT_EQ(buffer.OverlapAdd(samples), std::vector<float>({1.0f, 1.0f, 1.0f}));
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(buffer.OverlapAdd(samples),
              std::vector<float>({2.0f, 1.0f, 1.0f}));
  }
}

TEST(OLABufferTest, OverlapAdd4) {
  const size_t window_size = 4;
  const size_t hop_size = 4;
  const std::vector<float> samples = {1.0f, 1.0f, 1.0f, 1.0f};
  OLABuffer<float> buffer(window_size, hop_size);

  EXPECT_EQ(buffer.OverlapAdd(samples),
            std::vector<float>({1.0f, 1.0f, 1.0f, 1.0f}));
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(buffer.OverlapAdd(samples),
              std::vector<float>({1.0f, 1.0f, 1.0f, 1.0f}));
  }
}

TEST(OLABufferTest, OverlapAdd8) {
  const size_t window_size = 4;
  const size_t hop_size = 8;
  const std::vector<float> samples = {1.0f, 1.0f, 1.0f, 1.0f};
  OLABuffer<float> buffer(window_size, hop_size);

  EXPECT_EQ(buffer.OverlapAdd(samples),
            std::vector<float>({1.0f, 1.0f, 1.0f, 1.0f, 0.f, 0.f, 0.f, 0.f}));
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(buffer.OverlapAdd(samples),
              std::vector<float>({1.0f, 1.0f, 1.0f, 1.0f, 0.f, 0.f, 0.f, 0.f}));
  }
}

// Test the OLABuffer implementation but overlap adding a sinewave. Write the
// result to a file for validation.
TEST(OLABufferTest, OverlapAddSinewaveNoOverlap) {
  const size_t window_size = 1024;
  const size_t hop_size = 1024;
  const size_t num_samples = 48000;
  const float frequency = 440.0f;
  const float amplitude = .5f;
  const float sample_rate = 48000.f;

  // Generate sine wave samples.
  std::vector<float> sine_wave(num_samples);
  for (int i = 0; i < num_samples; ++i) {
    sine_wave[i] = amplitude * sin(2.0f * M_PI * frequency * i / sample_rate);
  }

  // Configure OLABuffer.
  OLABuffer<float> buffer(window_size, hop_size);

  // Output buffer.
  std::vector<float> output(num_samples);

  // Process sine wave in blocks of window_size.
  for (int sampsProcd = 0, output_samps_procd = 0; sampsProcd < num_samples;
       sampsProcd += window_size) {
    std::vector<float> curr_frame(window_size);
    for (int sample = 0; sample < window_size; ++sample) {
      curr_frame[sample] = sine_wave[sampsProcd + sample];
    }

    auto processedChunk = buffer.OverlapAdd(curr_frame);

    // Write processed samples to the output buffer.
    for (int sample = 0; sample < processedChunk.size(); ++sample) {
      output[output_samps_procd + sample] = processedChunk[sample];
    }
    output_samps_procd += hop_size;
  }

  // Write the output to a file for validation.
  AudioFile<float> output_file;
  output_file.setNumChannels(1);
  output_file.setNumSamplesPerChannel(num_samples);
  output_file.setSampleRate(sample_rate);
  output_file.samples[0] = output;
  ASSERT_TRUE(output_file.save("NOLA_sinewave.wav"));
}