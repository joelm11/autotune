#include "../resampler.h"

#include <gtest/gtest.h>

// Resample a 440Hz sine wave at 44.1kHz to 48kHz.
TEST(ResamplerTest, ResampleSine) {
  const int in_rate = 44100;
  const int out_rate = 48000;
  const int num_samples = 1000;
  Resampler resampler(in_rate, out_rate, num_samples);

  // Generate a 440Hz sine wave at 44.1kHz.
  std::vector<float> input(num_samples);
  for (int i = 0; i < num_samples; ++i) {
    input[i] = 0.5f * sinf(2 * M_PI * 440 * i / in_rate);
  }

  // Resample the sine wave to 48kHz.
  std::vector<float> output = resampler.Resample(input);

  // Check that the output has the expected number of samples.
  EXPECT_EQ(output.size(), num_samples * out_rate / in_rate);

  // Check that the output has the expected frequency (440Hz).
  // Only check the first half of the output samples as some zeros are added at
  // the end.
  const float kTolerance = 0.1f;
  for (int i = 0; i < num_samples / 2; ++i) {
    float expected = 0.5f * sinf(2 * M_PI * 440 * i / out_rate);
    ASSERT_NEAR(output[i], expected, kTolerance) << "Failed at sample " << i;
  }
}

// Resample a 440Hz sinewave at 48kHz to 44.1kHz.
TEST(ResamplerTest, ResampleSineDown) {
  const int in_rate = 48000;
  const int out_rate = 44100;
  const int num_samples = 1000;
  Resampler resampler(in_rate, out_rate, num_samples);

  // Generate a 440Hz sine wave at 48kHz.
  std::vector<float> input(num_samples);
  for (int i = 0; i < num_samples; ++i) {
    input[i] = 0.5f * sinf(2 * M_PI * 440 * i / in_rate);
  }

  // Resample the sine wave to 44.1kHz.
  std::vector<float> output = resampler.Resample(input);

  // Check that the output has the expected number of samples.
  EXPECT_EQ(output.size(), num_samples * out_rate / in_rate);

  // Check that the output has the expected frequency (440Hz).
  // Only check the first half of the output samples as some zeros are added at
  // the end.
  const float kTolerance = 0.1f;
  for (int i = 0; i < num_samples / 2; ++i) {
    float expected = 0.5f * sinf(2 * M_PI * 440 * i / out_rate);
    ASSERT_NEAR(output[i], expected, kTolerance) << "Failed at sample " << i;
  }
}

// Resample a 440Hz sinewave at 48kHz to 44.1kHz.
TEST(ResamplerTest, ResampleSineDownBufferSizing) {
  const int in_rate = 48000;
  const int out_rate = 44100;
  const int num_samples = 2048;

  // Generate a 440Hz sine wave at 48kHz.
  std::vector<float> input(num_samples);
  for (int i = 0; i < num_samples; ++i) {
    input[i] = 0.5f * sinf(2 * M_PI * 440 * i / in_rate);
  }

  const unsigned kNumBuffers = 64;
  const unsigned kInBuffSize = (float)num_samples / kNumBuffers;
  const unsigned kOutBuffSize = kInBuffSize * (float)out_rate / in_rate;
  Resampler resampler(in_rate, out_rate, kInBuffSize);
  for (int i = 0; i < num_samples; i += kInBuffSize) {
    // Grab an input buffer.
    std::vector<float> input_buffer(num_samples);
    for (int j = 0; j < num_samples; ++j) {
      input_buffer[j] = input[j + i];
    }

    // Resample the sine wave to 44.1kHz.
    std::vector<float> output = resampler.Resample(input_buffer);

    // Check that the output has the expected number of samples.
    EXPECT_EQ(output.size(), kOutBuffSize);

    for (int i = 0; i < output.size(); ++i) {
      std::cout << output[i] << std::endl;
    }
    std::cout << std::endl;
  }
}