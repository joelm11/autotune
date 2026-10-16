#ifndef PITCH_SHIFT_H
#define PITCH_SHIFT_H
#include "../external/speex_resampler/speex_resampler.h"
#include "../vocoder/vocoder.hh"
class PitchShifter {
 public:
  using SizeType = std::size_t;
  using FBuffer = std::vector<float>;

  PitchShifter(const int frame_size, const int sample_rate,
               const float scale_factor);

  FBuffer Process(const FBuffer& src);

 private:
  Vocoder vocoder;
  //   Resampler resampler;
};
#endif  // PITCH_SHIFT_H