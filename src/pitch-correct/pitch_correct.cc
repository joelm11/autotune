#include "pitch_correct.hh"

#include "../pitch-shift/pitch_shift.hh"

namespace {
// Shifter bookkeeping only; the applied resample ratio is rate-independent.
constexpr SizeType kSampleRate = 44100;
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
    // Seed the smoother at the first target so the initial block needs no ramp.
    smoother_ = std::make_unique<smoothing::OnePoleSmoother>(
        smoothing::kDefaultCoefficient, target_scale_factor);
  } else if (const auto smoothed = smoother_->Smooth(target_scale_factor)) {
    shifter_->SetScaleFactor(*smoothed);
  }
  return shifter_->Process(src);
}
