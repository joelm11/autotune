#include "pitch_shift.hh"

PitchShifter::PitchShifter(const SizeType frame_size,
                           const SizeType sample_rate, const float scale_factor)
    : resample_in_(static_cast<int>(frame_size * scale_factor)),
      scale_factor_(scale_factor),
      vocoder(frame_size, sample_rate, scale_factor),
      resampler(
          static_cast<int>(sample_rate),
          static_cast<int>(static_cast<float>(sample_rate) / scale_factor),
          resample_in_) {}

FBuffer PitchShifter::Process(const FBuffer& src) {
  auto stretched = vocoder.Process(src);
  if (stretched.empty()) {
    return {};
  }

  // Resample stretched back to exactly src.size() samples at the true
  // duration-preserving ratio (out/in = src/stretched).
  if (stretched.size() != static_cast<SizeType>(resample_in_) ||
      src.size() != static_cast<SizeType>(resampler.NumSamplesOut())) {
    resampler.SetRatio(static_cast<unsigned int>(src.size()),
                       static_cast<unsigned int>(stretched.size()));
    resampler.SetInOutSamples(static_cast<int>(stretched.size()),
                              static_cast<int>(src.size()));
  }

  return resampler.Resample(stretched);
}
