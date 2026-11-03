#ifndef PITCH_DETECT_H
#define PITCH_DETECT_H
#include "../api.hh"

class PitchDetect {
 public:
  struct PitchType {
    unsigned f0;
  };

  virtual PitchType Process(const FBuffer& src) = 0;
};
#endif  // PITCH_DETECT_H