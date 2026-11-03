#ifndef PITCH_SHIFT_H
#define PITCH_SHIFT_H

#include "../api.hh"
#include "../resampler/resampler.h"
#include "../vocoder/vocoder.hh"

class PitchShifter {
 public:
  PitchShifter(const SizeType frame_size, const SizeType sample_rate,
               const float scale_factor);

  FBuffer Process(const FBuffer& src);

 private:
  const int resample_in_;
  const float scale_factor_;
  Vocoder vocoder;
  Resampler resampler;
};
#endif  // PITCH_SHIFT_H
