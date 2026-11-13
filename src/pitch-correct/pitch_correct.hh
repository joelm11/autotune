#ifndef PITCH_CORRECT_H
#define PITCH_CORRECT_H
#include <memory>

#include "../api.hh"
#include "pitch.hh"
#include "smoothing.hh"

class PitchShifter;

class PitchCorrect {
 public:
  using Key = pitch::Key;

  PitchCorrect(const Key key, const float adjustment_rate);
  ~PitchCorrect();

  void SetKey(const Key new_key);

  FBuffer Process(const FBuffer& src, const float fund_freq);

 private:
  float ScaleFactor(const float fund_freq) const;

  float adjustment_rate_ = 1.f;
  Key key;
  std::unique_ptr<smoothing::Smoother> smoother_;
  std::unique_ptr<PitchShifter> shifter_;
};
#endif  // PITCH_CORRECT_H
