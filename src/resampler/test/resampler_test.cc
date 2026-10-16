#include "../resampler.h"

#include <gtest/gtest.h>

// Resample a 440Hz sine wave at 44.1kHz to 48kHz.
TEST(ResamplerTest, ResampleSine) {
  const int in_rate = 44100;
  const int out_rate = 48000;
  const int num_channels = 2;
  const int num_samples = 1000;
  Resampler resampler(in_rate, out_rate, num_channels, num_samples);

  // Generate a 440Hz sine wave at 44.1kHz.
  std::vector<std::vector<float>> input(num_channels,
                                        std::vector<float>(num_samples));
  for (int ch = 0; ch < num_channels; ++ch) {
    for (int i = 0; i < num_samples; ++i) {
      input[ch][i] = 0.5f * sinf(2 * M_PI * 440 * i / in_rate);
    }
  }

  // Resample the sine wave to 48kHz.
  std::vector<std::vector<float>> output = resampler.Resample(input);

  // Check that the output has the expected number of samples.
  for (int ch = 0; ch < num_channels; ++ch) {
    EXPECT_EQ(output[ch].size(), num_samples * out_rate / in_rate);
  }

  // Check that the output has the expected frequency (440Hz).
  // Only check the first half of the output samples as some zeros are added at
  // the end.
  const float kTolerance = 0.1f;
  for (int channel = 0; channel < num_channels; ++channel) {
    for (int i = 0; i < num_samples / 2; ++i) {
      float expected = 0.5f * sinf(2 * M_PI * 440 * i / out_rate);
      ASSERT_NEAR(output[channel][i], expected, kTolerance)
          << "Failed at channel " << channel << " sample " << i;
    }
  }
}

// Resample a 440Hz sinewave at 48kHz to 44.1kHz.
TEST(ResamplerTest, ResampleSineDown) {
  const int in_rate = 48000;
  const int out_rate = 44100;
  const int num_channels = 2;
  const int num_samples = 1000;
  Resampler resampler(in_rate, out_rate, num_channels, num_samples);

  // Generate a 440Hz sine wave at 48kHz.
  std::vector<std::vector<float>> input(num_channels,
                                        std::vector<float>(num_samples));
  for (int ch = 0; ch < num_channels; ++ch) {
    for (int i = 0; i < num_samples; ++i) {
      input[ch][i] = 0.5f * sinf(2 * M_PI * 440 * i / in_rate);
    }
  }

  // Resample the sine wave to 44.1kHz.
  std::vector<std::vector<float>> output = resampler.Resample(input);

  // Check that the output has the expected number of samples.
  for (int ch = 0; ch < num_channels; ++ch) {
    EXPECT_EQ(output[ch].size(), num_samples * out_rate / in_rate);
  }

  // Check that the output has the expected frequency (440Hz).
  // Only check the first half of the output samples as some zeros are added at
  // the end.
  const float kTolerance = 0.1f;
  for (int channel = 0; channel < num_channels; ++channel) {
    for (int i = 0; i < num_samples / 2; ++i) {
      float expected = 0.5f * sinf(2 * M_PI * 440 * i / out_rate);
      ASSERT_NEAR(output[channel][i], expected, kTolerance)
          << "Failed at channel " << channel << " sample " << i;
    }
  }
}

// Resample a 440Hz sinewave at 48kHz to 44.1kHz.
TEST(ResamplerTest, ResampleSineDownBufferSizing) {
  const int in_rate = 48000;
  const int out_rate = 44100;
  const int num_channels = 2;
  const int num_samples = 2048;

  // Generate a 440Hz sine wave at 48kHz.
  std::vector<std::vector<float>> input(num_channels,
                                        std::vector<float>(num_samples));
  for (int ch = 0; ch < num_channels; ++ch) {
    for (int i = 0; i < num_samples; ++i) {
      input[ch][i] = 0.5f * sinf(2 * M_PI * 440 * i / in_rate);
    }
  }

  const unsigned kNumBuffers = 64;
  const unsigned kInBuffSize = (float)num_samples / kNumBuffers;
  const unsigned kOutBuffSize = kInBuffSize * (float)out_rate / in_rate;
  Resampler resampler(in_rate, out_rate, num_channels, kInBuffSize);
  for (int i = 0; i < num_samples; i += kInBuffSize) {
    // Grab an input buffer.
    std::vector<std::vector<float>> input_buffer(
        num_channels, std::vector<float>(num_samples));
    for (int ch = 0; ch < num_channels; ++ch) {
      for (int j = 0; j < num_samples; ++j) {
        input_buffer[ch][j] = input[ch][j + i];
      }
    }

    // Resample the sine wave to 44.1kHz.
    std::vector<std::vector<float>> output = resampler.Resample(input_buffer);

    // Check that the output has the expected number of samples.
    for (int ch = 0; ch < num_channels; ++ch) {
      EXPECT_EQ(output[ch].size(), kOutBuffSize);
    }

    for (int i = 0; i < output[0].size(); ++i) {
      std::cout << output[0][i] << std::endl;
    }
    std::cout << std::endl;
  }
}