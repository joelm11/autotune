#include "pitch_correct.hh"

#include "../pitch-shift/pitch_shift.hh"

namespace {
// Sample rate used internally by the pitch shifter. It only carries into the
// vocoder/resampler bookkeeping; the actual resample ratio applied is
// sample-rate independent, so this only needs to be a sane non-zero value.
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

  const float scale_factor = ScaleFactor(fund_freq);

  if (!shifter_) {
    shifter_ =
        std::make_unique<PitchShifter>(src.size(), kSampleRate, scale_factor);
    last_scale_factor_ = scale_factor;
  } else if (scale_factor != last_scale_factor_) {
    shifter_->SetScaleFactor(scale_factor);
    last_scale_factor_ = scale_factor;
  }

  return shifter_->Process(src);
}
