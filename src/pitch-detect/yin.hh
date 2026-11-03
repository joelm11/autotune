#ifndef YIN_H
#define YIN_H
#include <cmath>

#include "pitch_detect.hh"

class Yin : public PitchDetect {
 public:
  Yin(const SizeType /*frame_sz*/) {}

  PitchDetect::PitchType Process(const FBuffer& src) override {
    const SizeType N = src.size();
    if (N < 2) {
      return PitchType{0};
    }
    const unsigned kFs = 16000;

    // Difference function: d(lag) = sum_j (src[j] - src[j+lag])^2
    FBuffer diff(N, 0.f);
    for (SizeType lag = 0; lag < N; ++lag) {
      float sum = 0.f;
      for (SizeType j = 0; j + lag < N; ++j) {
        const float d = src[j] - src[j + lag];
        sum += d * d;
      }
      diff[lag] = sum;
    }

    // Cumulative mean normalized difference (CMND).
    FBuffer cmnd(N, 1.f);
    float running = 0.f;
    for (SizeType lag = 1; lag < N; ++lag) {
      running += diff[lag];
      cmnd[lag] =
          running > 0.f ? diff[lag] * static_cast<float>(lag) / running : 1.f;
    }

    // Restrict search to a plausible fundamental-frequency range.
    const unsigned kMinFreq = 60;    // Hz
    const unsigned kMaxFreq = 3500;  // Hz
    const SizeType kMinLag = kFs / kMaxFreq + 1;
    const SizeType kMaxLag = kFs / kMinFreq + 1;
    const SizeType kLast = std::min(kMaxLag, N - 1);

    const float kThresh = 0.1f;
    for (SizeType lag = kMinLag; lag < kLast; ++lag) {
      if (cmnd[lag] < kThresh) {
        // Walk to the local minimum.
        while (lag < N - 1 && cmnd[lag] > cmnd[lag + 1]) {
          ++lag;
        }

        // Parabolic interpolation for sub-sample lag precision.
        float refined = static_cast<float>(lag);
        if (lag > 0 && lag < N - 1) {
          const float y0 = cmnd[lag - 1];
          const float y1 = cmnd[lag];
          const float y2 = cmnd[lag + 1];
          const float denom = y0 - 2.f * y1 + y2;
          if (denom != 0.f) {
            refined = static_cast<float>(lag) + 0.5f * (y0 - y2) / denom;
          }
        }

        const float f0 = static_cast<float>(kFs) / refined;
        return PitchType{static_cast<unsigned>(std::lround(f0))};
      }
    }

    return PitchType{0};
  }
};
#endif  // YIN_H