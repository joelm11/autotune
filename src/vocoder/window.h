#ifndef VOCODER_WINDOW_H
#define VOCODER_WINDOW_H

#include <array>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <vector>

template <typename T>
std::vector<T> ComputeHannWin(const std::size_t win_size,
                              const std::size_t an_hop_size) {
  std::vector<T> window(win_size);
  for (std::size_t i = 0; i < win_size; ++i) {
    window[i] = 0.5 * (1 - std::cos(2 * std::numbers::pi * i / (win_size - 1)));
  }
  return window;
}
#endif