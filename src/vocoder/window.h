#ifndef VOCODER_WINDOW_H
#define VOCODER_WINDOW_H

#include <cmath>
#include <cstddef>
#include <numbers>
#include <vector>

// Periodic Hann, normalized so overlap-adding its square at `hop_size` sums
// to exactly 1. The vocoder applies the window twice (analysis + synthesis),
// so this keeps the output envelope flat with unity gain.
template <typename T>
std::vector<T> ComputeHannWin(const std::size_t win_size,
                              const std::size_t hop_size) {
  // Hann window fx
  std::vector<T> hann(win_size);
  for (std::size_t i = 0; i < win_size; ++i)
    hann[i] = 0.5 * (1 - std::cos(2 * std::numbers::pi * i / win_size));

  std::vector<T> window(win_size);
  for (std::size_t i = 0; i < win_size; ++i) {
    // Sum hann^2 over the frames (hop_size apart, m<0 = earlier frames) whose
    // window covers sample i, then normalize.
    const long i_l = static_cast<long>(i);
    const long win_l = static_cast<long>(win_size);
    const long hop_l = static_cast<long>(hop_size);
    const long m_lo = -(win_l / hop_l) - 1;  // first frame touching i
    const long m_hi = i_l / hop_l;           // last frame touching i

    T sum = 0;
    for (long m = m_lo; m <= m_hi; ++m) {
      const long idx = i_l - m * hop_l;
      if (idx >= 0 && idx < win_l) sum += hann[idx] * hann[idx];
    }
    window[i] = hann[i] / std::sqrt(sum);
  }
  return window;
}
#endif  // VOCODER_WINDOW_H
