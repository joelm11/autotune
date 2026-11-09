#ifndef PITCH_CORRECT_H
#define PITCH_CORRECT_H
#include "../api.hh"

class PitchCorrect {
 public:
  struct PitchCorrectType {
    unsigned octave;
    float correction_factor;
  };

  enum Key { C, G, D, A, E };

  PitchCorrect(const Key key, const float adjustment_rate) {}

  FBuffer Process(const FBuffer& src, const float fund_freq);

 private:
  unsigned Octave(const float fund_freq);
  unsigned NearestNoteFreq(const float fund_freq);
  std::vector<float> ScaleNoteFreqs(const unsigned octave, const Key key);

  Key key;
  float adjustment_rate = 1.f;
};
#endif  // PITCH_CORRECT_H
