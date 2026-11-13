#include "vocoder_impl.hh"

#include <algorithm>

#include "../utils/ola_buffer.h"
#include "../utils/window.hh"
#include "utils.hh"

VocoderImpl::VocoderImpl(const SizeType frame_sz, const SizeType sample_rate,
                         const float scale_factor)
    : kFrameSz_(frame_sz),
      kSampleRate_(sample_rate),
      kAnalysisHopSize(frame_sz / kAnalysisHops),
      kSynthesisHopSize_(kAnalysisHopSize * scale_factor),
      kAnalysisWindow_(ComputeHannWin<float>(kFrameSz_, kAnalysisHopSize)),
      kSynthesisWindow_(ComputeHannWin<float>(kFrameSz_, kSynthesisHopSize_)),
      input_acc_(frame_sz),
      prev_frame_phase_(kFrameSz_, 0.0f),
      prev_frame_synth_phase_(kFrameSz_, 0.0f),
      prev_frame_peaks_(kFrameSz_, false),
      output_acc_(frame_sz, kSynthesisHopSize_) {
  fft_ = std::make_unique<FFTImpl>(frame_sz);
}

void VocoderImpl::SetScaleFactor(const float scale_factor) {
  kSynthesisHopSize_ = kAnalysisHopSize * scale_factor;
  kSynthesisWindow_ = ComputeHannWin<float>(kFrameSz_, kSynthesisHopSize_);
  output_acc_.SetHopSize(kSynthesisHopSize_);
}

FBuffer VocoderImpl::Process(const FBuffer& src) {
  input_acc_.Write(src);

  if (!input_acc_.Full()) {
    return {};
  }

  // Get source frames
  std::vector<FBuffer> src_frames;
  while (input_acc_.Full()) {
    src_frames.push_back(input_acc_.Read(kAnalysisHopSize));
  }

  FBuffer output_frame;
  FBuffer prev_frame_phase = prev_frame_phase_;
  FBuffer prev_frame_synth_phase = prev_frame_synth_phase_;
  std::vector<bool> prev_frame_peaks = prev_frame_peaks_;
  for (auto& src_frame : src_frames) {
    CFBuffer analysis_frame(kFrameSz_);
    FBuffer procd_samples(kSynthesisHopSize_);

    Analysis(src_frame, analysis_frame);
    ModifyPhaseT(analysis_frame, prev_frame_phase, prev_frame_synth_phase,
                 prev_frame_peaks);
    procd_samples = Synthesis(analysis_frame, output_acc_);

    output_frame.insert(output_frame.end(), procd_samples.begin(),
                        procd_samples.end());
  }

  prev_frame_phase_ = prev_frame_phase;
  prev_frame_synth_phase_ = prev_frame_synth_phase;
  prev_frame_peaks_ = prev_frame_peaks;

  return output_frame;
}

void VocoderImpl::Analysis(FBuffer& src, CFBuffer& out) {
  // Apply window function.
  for (SizeType i = 0; i < kFrameSz_; ++i) {
    src[i] *= kAnalysisWindow_[i];
  }
  // Convert FFT result to magnitude and phase format.
  fft_->Forward(src, out);
  for (SizeType i = 0; i < kFrameSz_; ++i) {
    out[i] = std::complex<float>(std::abs(out[i]), std::arg(out[i]));
  }
}

std::vector<SizeType> VocoderImpl::FindPeaks(const CFBuffer& an_frame) const {
  const SizeType kNyquist = kFrameSz_ / 2;

  float max_mag = 0.0f;
  for (SizeType i = 0; i < kFrameSz_; ++i) {
    max_mag = std::max(max_mag, an_frame[i].real());
  }
  const float peak_threshold = kPeakThresholdRatio * max_mag;

  std::vector<SizeType> peaks;
  peaks.reserve(kNyquist / 2);
  for (SizeType k = 1; k < kNyquist; ++k) {
    const float mag_k = an_frame[k].real();
    if (mag_k > an_frame[k - 1].real() && mag_k >= an_frame[k + 1].real() &&
        mag_k > peak_threshold) {
      peaks.push_back(k);
    }
  }
  return peaks;
}

void VocoderImpl::ModifyPhaseT(CFBuffer& an_frame, FBuffer& prev_frame_phase,
                               FBuffer& prev_frame_synth_phase,
                               std::vector<bool>& prev_frame_peaks) {
  const SizeType kNyquist = kFrameSz_ / 2;

  FBuffer analysis_phase(kFrameSz_);
  for (SizeType i = 0; i < kFrameSz_; ++i) {
    analysis_phase[i] = an_frame[i].imag();
  }

  FBuffer bin_freqs(kFrameSz_), phase_advance(kFrameSz_),
      inter_result(kFrameSz_);
  // Constants.
  for (SizeType i = 0; i < kFrameSz_; ++i) {
    bin_freqs[i] = 2 * M_PI * i / kFrameSz_;
    phase_advance[i] = bin_freqs[i] * kAnalysisHopSize;
  }
  // Wrapped phase advance.
  for (SizeType i = 0; i < kFrameSz_; ++i) {
    inter_result[i] =
        an_frame[i].imag() - prev_frame_phase[i] - phase_advance[i];
    inter_result[i] =
        inter_result[i] - 2 * M_PI * round(inter_result[i] / (2 * M_PI));
    prev_frame_phase[i] = an_frame[i].imag();
  }
  // Instantaneous frequency.
  for (SizeType i = 0; i < kFrameSz_; ++i) {
    inter_result[i] = bin_freqs[i] + inter_result[i] / kAnalysisHopSize;
  }
  // Phase propagation and synthesis with wrapping.
  for (SizeType i = 0; i < kFrameSz_; ++i) {
    inter_result[i] =
        prev_frame_synth_phase[i] + kSynthesisHopSize_ * inter_result[i];
    inter_result[i] =
        inter_result[i] - 2 * M_PI * round(inter_result[i] / (2 * M_PI));
    an_frame[i].imag(inter_result[i]);
    prev_frame_synth_phase[i] = inter_result[i];
  }

  // ---- Phase locking ----
  // Find the spectral peaks on the positive-frequency half; the negative
  // half is a mirror image and is reconstructed symmetrically at the end.
  const std::vector<SizeType> peaks = FindPeaks(an_frame);
  std::vector<bool> frame_peaks(kFrameSz_, false);
  for (SizeType p : peaks) {
    frame_peaks[p] = true;
  }

  if (!peaks.empty()) {
    for (SizeType k = 1; k < kNyquist; ++k) {
      // Assign the bin to its nearest analysis peak (ties resolve toward the
      // lower-frequency peak).
      SizeType nearest = peaks.front();
      SizeType nearest_dist = kFrameSz_;
      for (SizeType p : peaks) {
        const SizeType dist = p > k ? p - k : k - p;
        if (dist < nearest_dist) {
          nearest_dist = dist;
          nearest = p;
        }
      }

      const bool new_peak = !prev_frame_peaks[nearest];
      const float peak_phase =
          new_peak ? analysis_phase[nearest] : an_frame[nearest].imag();

      // Lock this bin's phase to the peak while keeping the within-peak
      // offsets of the analysis frame, so the partial's wave shape survives
      // the time scaling.
      float locked = peak_phase + analysis_phase[k] - analysis_phase[nearest];
      locked -= 2 * M_PI * round(locked / (2 * M_PI));
      an_frame[k].imag(locked);
    }
  }
  prev_frame_peaks.swap(frame_peaks);

  // Mirror the (locked) positive-frequency phases onto the negative half so
  // the output spectrum.
  for (SizeType k = 1; k < kNyquist; ++k) {
    an_frame[kFrameSz_ - k].imag(-an_frame[k].imag());
  }

  // Re-seed the per-bin accumulation with actual phases.
  for (SizeType i = 0; i < kFrameSz_; ++i) {
    prev_frame_synth_phase[i] = an_frame[i].imag();
  }
}

FBuffer VocoderImpl::Synthesis(CFBuffer& src, OLABuffer<float>& out) {
  const auto kNumSamples_ = src.size();
  FBuffer output_buffer_(kNumSamples_);
  // Convert magnitude and phase back to complex format.
  for (SizeType j = 0; j < kNumSamples_; ++j) {
    src[j] = std::polar(src[j].real(), src[j].imag());
  }
  // Inverse FFT.
  fft_->Inverse(src, output_buffer_);
  // Apply window function.
  for (SizeType j = 0; j < kNumSamples_; ++j) {
    output_buffer_[j] *= kSynthesisWindow_[j] / kNumSamples_;
  }
  // Overlap-add.
  return out.OverlapAdd(output_buffer_);
}