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

class PitchDetectTest : public testing::Test {
 protected:
  PitchDetectTest() {
    auto s0 = GenSine(kSr, kFundFreqs[0], kFrameSize);
    auto s1 = GenSine(kSr, kFundFreqs[1], kFrameSize);
    auto s2 = s0;
    for (int i = 0; i < s1.size(); ++i) {
      s2[i] += .1f * s1[i];
    }

    signals.push_back(s0);
    signals.push_back(s1);
    signals.push_back(s2);
  }

  const unsigned kSr = 16e3;
  const unsigned kFrameSize = 1024;
  const std::vector<unsigned> kFundFreqs = {440, 153, 440};
  std::vector<std::vector<float>> signals;
};

TEST_F(PitchDetectTest, Yin) {
  Yin pd(kSr, kFrameSize);

  for (int i = 0; i < signals.size(); ++i) {
    unsigned fund_freq = pd.Process(signals[i]).f0;
    EXPECT_EQ(fund_freq, kFundFreqs[i])
        << "Calculated incorrect fundamental frequency: " << fund_freq
        << ". Expected: " << kFundFreqs[i];
  }
}