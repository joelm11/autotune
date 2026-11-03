#include <gtest/gtest.h>

#include <numbers>

#include "yin.hh"

std::vector<float> GenSine(const int sr, const float f, const int num_samples) {
  std::vector<float> out(num_samples);
  for (int i = 0; i < num_samples; ++i) {
    out[i] = std::sin(2 * std::numbers::pi * i * f / sr);
  }
  return out;
}

TEST(PitchDetectTest, Yin) { Yin foo(1024); }
// Test template:
// Construct a pitch detection object
// For a list of different sources, ensure f0 detection is correct