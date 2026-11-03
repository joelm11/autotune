#ifndef YIN_H
#define YIN_H
#include <memory>
#include <numeric>

#include "../vocoder/fft/fft_impl.h"
#include "pitch_detect.hh"

class Yin : public PitchDetect {
 public:
  Yin(const SizeType frame_sz) : fft_(std::make_unique<FFTImpl>(frame_sz)) {}

  PitchDetect::PitchType Process(const FBuffer& src) override {
    const SizeType N = src.size();
    FBuffer corrs(N), diff(N);
    CFBuffer fcorrs_lagged(N);

    // Cumulative sum of squares
    float sum = 0;
    for (int i = 0; i < N; ++i) {
      corrs[i] = src[i] * src[i] + sum;
      sum = corrs[i];
    }

    // Autocorrelation
    fft_->Forward(src, fcorrs_lagged);
    for (int i = 0; i < N; ++i) {
      fcorrs_lagged[i].real(std::abs(fcorrs_lagged[i]) / N);
    }

    // Difference function
    diff[0] = 2 * corrs[N - 1] - 2 * fcorrs_lagged[0].real();
    for (int i = 1; i < N; ++i) {
      diff[i] = 2 * corrs[N - 1] - corrs[i - 1] - 2 * fcorrs_lagged[i].real();
    }

    // Normalize
    const float kMean = std::accumulate(diff.begin(), diff.end(), 0.f) / N;
    const float kSqSum = std::inner_product(
        diff.begin(), diff.end(), diff.begin(), 0.f, std::plus<>(),
        [kMean](float a, float b) { return (a - kMean) * (b - kMean); });
    const float kStd = std::sqrt(kSqSum / N);
    for (int i = 0; i < N; ++i) {
      diff[i] = (diff[i] - kMean) / kStd;
    }

    // Find minimum below some threshold
    const unsigned kFs = 16e3;
    const unsigned max_freq_ind = kFs / 3500 + 1;
    const float kThresh = 1.f;
    for (int lag = max_freq_ind; lag < N; ++lag) {
      if (diff[lag] < kThresh) {
        while (diff[lag] > diff[lag + 1]) {
          ++lag;
        }
        return PitchType(kFs / (lag + 1));
      }
    }

    return PitchType(-1);
  }

 private:
  std::unique_ptr<FFTImpl> fft_;
};
#endif  // YIN_H