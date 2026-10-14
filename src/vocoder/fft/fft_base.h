#ifndef API_FFT_FFT_BASE_H
#define API_FFT_FFT_BASE_H

#include <complex>
#include <cstddef>
#include <vector>

template <typename T>
class FFTBase {
 public:
  virtual ~FFTBase() = default;

  virtual void Forward(const std::vector<T>& input,
                       std::vector<std::complex<T>>& output) = 0;

  virtual void Inverse(const std::vector<std::complex<T>>& input,
                       std::vector<T>& output) = 0;
};

#endif