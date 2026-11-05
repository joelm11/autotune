#include "vocoder_impl.hh"

#include "../utils/ola_buffer.h"
#include "utils.hh"
#include "../utils/window.hh"

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
      output_acc_(frame_sz, kSynthesisHopSize_) {
  fft_ = std::make_unique<FFTImpl>(frame_sz);
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
  for (auto& src_frame : src_frames) {
    CFBuffer analysis_frame(kFrameSz_);
    FBuffer procd_samples(kSynthesisHopSize_);

    Analysis(src_frame, analysis_frame);
    ModifyPhaseT(analysis_frame, prev_frame_phase, prev_frame_synth_phase);
    procd_samples = Synthesis(analysis_frame, output_acc_);

    output_frame.insert(output_frame.end(), procd_samples.begin(),
                        procd_samples.end());
  }

  prev_frame_phase_ = prev_frame_phase;
  prev_frame_synth_phase_ = prev_frame_synth_phase;

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

void VocoderImpl::ModifyPhaseT(CFBuffer& an_frame, FBuffer& prev_frame_phase,
                               FBuffer& prev_frame_synth_phase) {
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
  // Phase propagation and synthesis.
  for (SizeType i = 0; i < kFrameSz_; ++i) {
    inter_result[i] =
        prev_frame_synth_phase[i] + kSynthesisHopSize_ * inter_result[i];
    an_frame[i].imag(inter_result[i]);
    prev_frame_synth_phase[i] = inter_result[i];
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