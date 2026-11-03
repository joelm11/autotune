#ifndef VOCODER_WINDOW_H
#define VOCODER_WINDOW_H

#include <cmath>
#include <numbers>

#include "../api.hh"

template <typename T>
std::vector<T> ComputeHannWin(const SizeType win_size,
                              const SizeType hop_size) {
  // Hann window fx
  std::vector<T> hann(win_size);
  for (SizeType i = 0; i < win_size; ++i)
    hann[i] = 0.5 * (1 - std::cos(2 * std::numbers::pi * i / win_size));

  std::vector<T> window(win_size);
  for (SizeType i = 0; i < win_size; ++i) {
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
