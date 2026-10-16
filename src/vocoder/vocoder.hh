#ifndef VOCODER_H
#define VOCODER_H

#include <vector>

#include "fft/fft_impl.h"

class VocoderImpl;

class Vocoder {
 public:
  using SizeType = std::size_t;
  using FBuffer = std::vector<std::vector<float>>;
  using CFBuffer = std::vector<std::vector<std::complex<float>>>;

  enum Effect {
    kNone = 0,
    kRobotize = 1,
    kTimeStretch = 2,
  };

  Vocoder(const SizeType num_channels, const SizeType sample_rate);

  // Applies input frame
  FBuffer Process(const FBuffer& src, const Effect effect);

 private:
  std::unique_ptr<VocoderImpl> impl_;
};

#endif  // VOCODER_H