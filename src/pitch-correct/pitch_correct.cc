#include "pitch_correct.hh"

#include <cmath>

#include "../pitch-shift/pitch_shift.hh"

namespace {
// Sample rate used internally by the pitch shifter. It only carries into the
// vocoder/resampler bookkeeping; the actual resample ratio applied is
// sample-rate independent, so this only needs to be a sane non-zero value.
constexpr SizeType kSampleRate = 44100;

// One-pole smoothing coefficient applied to scale-factor changes (per block).
// When the detected fundamental jumps onto a new scale note the ratio would
// otherwise snap instantly, which abruptly changes the vocoder's synthesis
// hop size between blocks and produces a discontinuity/click. Ramping it over
// a few blocks - combined with the vocoder preserving its running phase - lets
// the output glide between notes instead of stepping.
constexpr float kScaleSmoothing = 0.8f;

// Below this ratio delta there is no audible benefit to pushing a new value
// into the shifter; it just keeps re-setting the (noisy, converging) ratio on
// every block once the one-pole recursion has practically settled.
constexpr float kMinRatioDelta = 1e-6f;
}  // namespace

PitchCorrect::PitchCorrect(const Key key, const float adjustment_rate)
    : key(key), adjustment_rate_(adjustment_rate) {}

PitchCorrect::~PitchCorrect() = default;

void PitchCorrect::SetKey(const Key new_key) {
  if (new_key == key) {
    return;
  }
  key = new_key;
}

float PitchCorrect::ScaleFactor(const float fund_freq) const {
  // Ratio required to take the detected fundamental onto the nearest note of
  // the selected scale. adjustment_rate_ blends between "no correction" (0)
  // and "full correction" (1), and beyond 1 for over-correction.
  const float nearest = pitch::NearestNoteFreq(fund_freq, key);
  const float ratio = nearest / fund_freq;
  return 1.f + (ratio - 1.f) * adjustment_rate_;
}

FBuffer PitchCorrect::Process(const FBuffer& src, const float fund_freq) {
  if (src.empty() || fund_freq <= 0.f) {
    return {};
  }

  const float target_scale_factor = ScaleFactor(fund_freq);
  if (!shifter_) {
    shifter_ = std::make_unique<PitchShifter>(src.size(), kSampleRate,
                                              target_scale_factor);
    last_scale_factor_ = target_scale_factor;
  } else {
    // Smoothly ramp the scale factor toward its target. Preserving the
    // vocoder's running phase across these bounded changes keeps the output
    // phase-continuous; the one-pole ramp spreads each note transition over a
    // few blocks so the pitch glides rather than snaps.
    const float smoothed =
        last_scale_factor_ +
        kScaleSmoothing * (target_scale_factor - last_scale_factor_);
    if (std::fabs(smoothed - last_scale_factor_) > kMinRatioDelta) {
      shifter_->SetScaleFactor(smoothed);
      last_scale_factor_ = smoothed;
    }
  }
  return shifter_->Process(src);
}
