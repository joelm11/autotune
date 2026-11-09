#ifndef PITCH_CORRECT_H
#define PITCH_CORRECT_H
#include <memory>

#include "../api.hh"
#include "pitch.hh"

class PitchShifter;

class PitchCorrect {
 public:
  using Key = pitch::Key;

  PitchCorrect(const Key key, const float adjustment_rate);
  ~PitchCorrect();

  FBuffer Process(const FBuffer& src, const float fund_freq);

 private:
  // Ratio required to take the detected fundamental onto the nearest note of
  // the selected scale.
  float ScaleFactor(const float fund_freq) const;

  Key key;
  float adjustment_rate_ = 1.f;
  float last_scale_factor_ = 1.f;
  std::unique_ptr<PitchShifter> shifter_;
};
#endif  // PITCH_CORRECT_H
