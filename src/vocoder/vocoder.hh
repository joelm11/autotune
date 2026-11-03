#ifndef VOCODER_H
#define VOCODER_H

#include "../api.hh"

class VocoderImpl;

class Vocoder {
 public:
  Vocoder(const SizeType frame_size, const SizeType sample_rate,
          const float scale_factor);
  ~Vocoder();

  FBuffer Process(const FBuffer& src);

 private:
  std::unique_ptr<VocoderImpl> impl_;
};

#endif  // VOCODER_H