#include <complex>
#include <cstddef>
#include <vector>

#include "fft_base.h"
#include "kiss_fft.h"

class FFTImpl : public FFTBase<float> {
 public:
  using SizeType = std::size_t;

  FFTImpl(const SizeType fft_len) {
    forward_cfg_ = kiss_fft_alloc(fft_len, 0, nullptr, nullptr);
    inv_cfg_ = kiss_fft_alloc(fft_len, 1, nullptr, nullptr);
    input_buffer_.resize(fft_len);
    output_buffer_.resize(fft_len);
  }

  ~FFTImpl() {
    kiss_fft_free(forward_cfg_);
    kiss_fft_free(inv_cfg_);
  }

  void Forward(const std::vector<float>& input,
               std::vector<std::complex<float>>& output) override {
    // Copy real inputs to a complex vector.
    for (SizeType i = 0; i < input.size(); ++i) {
      input_buffer_[i].r = input[i];
      input_buffer_[i].i = 0.0f;
    }

    kiss_fft(forward_cfg_, input_buffer_.data(),
             reinterpret_cast<kiss_fft_cpx*>(output.data()));
  }

  void Inverse(const std::vector<std::complex<float>>& input,
               std::vector<float>& output) override {
    kiss_fft(inv_cfg_, reinterpret_cast<const kiss_fft_cpx*>(input.data()),
             output_buffer_.data());

    // Copy complex outputs to real vector.
    for (SizeType i = 0; i < output.size(); ++i) {
      output[i] = output_buffer_[i].r;
    }
  }

 private:
  kiss_fft_cfg forward_cfg_, inv_cfg_;
  std::vector<kiss_fft_cpx> input_buffer_;
  std::vector<kiss_fft_cpx> output_buffer_;
};