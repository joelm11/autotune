#include "pitch_shift.hh"

PitchShifter::PitchShifter(const int frame_size, const int sample_rate,
                           const float scale_factor)
    : vocoder(frame_size, sample_rate, scale_factor),
      resampler(sample_rate, sample_rate, frame_size) {}

PitchShifter::FBuffer PitchShifter::Process(const FBuffer& src) { return {}; }