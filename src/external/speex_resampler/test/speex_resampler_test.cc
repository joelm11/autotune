#include "../speex_resampler.h"

#include <gtest/gtest.h>

TEST(SpeexResamplerTest, SpeexResamplerState) {
  SpeexResamplerState* st = speex_resampler_init(1, 44100, 48000, 10, NULL);
  ASSERT_TRUE(st != NULL);
  speex_resampler_destroy(st);
}

TEST(SpeexResamplerTest, SpeexResampleSine) {
  // Create a buffer of a 440Hz sinewave sampled at 16kHz.
  const int kNumSamples = 32000;
  const int kSampleRate = 16000;
  const int kFrequency = 440;
  const float kAmplitude = 0.5;
  auto input = std::unique_ptr<float[]>(new float[kNumSamples]);
  for (int i = 0; i < kNumSamples; ++i) {
    input[i] = kAmplitude * sin(2 * M_PI * kFrequency * i / kSampleRate);
  }

  // Create a resampler with input rate 16kHz and output rate 48kHz.
  SpeexResamplerState* st =
      speex_resampler_init(1, kSampleRate, 48000, 10, NULL);
  ASSERT_TRUE(st != NULL);

  // Allocate an output buffer.
  const int kOutputSize = kNumSamples * 48000 / 16000 + 1;
  auto output = std::unique_ptr<float[]>(new float[kOutputSize]);

  // Resample the input buffer.
  uint32_t in_len = kNumSamples;
  uint32_t out_len = kOutputSize;
  int ret = speex_resampler_process_float(st, 0, input.get(), &in_len,
                                          output.get(), &out_len);
  ASSERT_EQ(0, ret);

  //   // OPTIONAL: Write the output buffer to STDOUT for plotting.
  //   for (int i = 200; i < 1000; ++i) {
  //     printf("%f\n", output[i]);
  //   }
}