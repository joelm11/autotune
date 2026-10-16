#include "vocoder_impl.hh"

#include "ola_buffer.h"
#include "utils.hh"
#include "window.h"

VocoderImpl::VocoderImpl(const SizeType frame_sz, const SizeType sample_rate,
                         const float scale_factor)
    : kFrameSz_(frame_sz),
      kSampleRate_(sample_rate),
      kAnalysisHopSize(frame_sz / kAnalysisHops),
      kSynthesisHopSize_(kAnalysisHopSize * scale_factor),
      kWindowBuffer_(ComputeHannWin<float>(kFrameSz_, kAnalysisHopSize)),
      input_acc_(frame_sz),
      output_acc_(frame_sz, kSynthesisHopSize_) {
  fft_ = std::make_unique<FFTImpl>(frame_sz);
}

VocoderImpl::FBuffer VocoderImpl::Process(const VocoderImpl::FBuffer& src) {
  input_acc_.Write(src);

  if (!input_acc_.Full()) {
    return {};
  }

  FBuffer output_frame;
  std::vector<FBuffer> src_frames = std::vector<FBuffer>(kAnalysisHops);
  std::vector<CFBuffer> analysis_frames = std::vector<CFBuffer>(kAnalysisHops);
  std::vector<FBuffer> prev_phase_frames,
      prev_synth_phase_frames = std::vector<FBuffer>(kAnalysisHops);

  prev_phase_frames[0] = prev_frame_phase_;
  prev_synth_phase_frames[0] = prev_frame_synth_phase_;
  for (int i = 0; i < kAnalysisHops; ++i) {
    FBuffer procd_samples(kSynthesisHopSize_);

    Analysis(src_frames[i], analysis_frames[i]);
    ModifyPhaseT(analysis_frames[i], prev_phase_frames[i],
                 prev_synth_phase_frames[i]);
    procd_samples = Synthesis(analysis_frames[i], output_acc_);

    output_frame.insert(output_frame.end(), procd_samples.begin(),
                        procd_samples.end());
  }

  prev_frame_phase_ = prev_phase_frames.back();
  prev_frame_synth_phase_ = prev_synth_phase_frames.back();

  return output_frame;
}

void VocoderImpl::Analysis(VocoderImpl::FBuffer& src,
                           VocoderImpl::CFBuffer& out) {
  // Apply window function.
  for (int i = 0; i < kFrameSz_; ++i) {
    src[i] *= kWindowBuffer_[i];
  }
  // Convert FFT result to magnitude and phase format.
  fft_->Forward(src, out);
  for (int i = 0; i < kFrameSz_; ++i) {
    out[i] = std::complex<float>(std::abs(out[i]), std::arg(out[i]));
  }
}

void VocoderImpl::ModifyPhaseT(VocoderImpl::CFBuffer& an_frame,
                               VocoderImpl::FBuffer& prev_frame_phase,
                               VocoderImpl::FBuffer& prev_frame_synth_phase) {
  FBuffer bin_freqs(kFrameSz_), phase_advance(kFrameSz_),
      inter_result(kFrameSz_);
  // Constants.
  for (int i = 0; i < kFrameSz_; ++i) {
    bin_freqs[i] = 2 * M_PI * i / kFrameSz_;
    phase_advance[i] = bin_freqs[i] * kAnalysisHopSize;
  }
  // Wrapped phase advance.
  for (int i = 0; i < kFrameSz_; ++i) {
    inter_result[i] =
        an_frame[i].imag() - prev_frame_phase[i] - phase_advance[i];
    inter_result[i] =
        inter_result[i] - 2 * M_PI * round(inter_result[i] / (2 * M_PI));
    prev_frame_phase[i] = an_frame[i].imag();
  }
  // Instantaneous frequency.
  for (int i = 0; i < kFrameSz_; ++i) {
    inter_result[i] = bin_freqs[i] + inter_result[i] / kAnalysisHopSize;
  }
  // Phase propagation and synthesis.
  for (int i = 0; i < kFrameSz_; ++i) {
    inter_result[i] =
        prev_frame_synth_phase[i] + kSynthesisHopSize_ * inter_result[i];
    an_frame[i].imag(inter_result[i]);
    prev_frame_synth_phase[i] = inter_result[i];
  }
}

VocoderImpl::FBuffer VocoderImpl::Synthesis(VocoderImpl::CFBuffer& src,
                                            OLABuffer<float>& out) {
  const auto kNumSamples_ = src.size();
  FBuffer output_buffer_(kNumSamples_);
  // Convert magnitude and phase back to complex format.
  for (int j = 0; j < kNumSamples_; ++j) {
    src[j] = std::polar(src[j].real(), src[j].imag());
  }
  // Inverse FFT.
  fft_->Inverse(src, output_buffer_);
  // Apply window function.
  for (int j = 0; j < kNumSamples_; ++j) {
    output_buffer_[j] *= kWindowBuffer_[j] / kNumSamples_;
  }
  // Overlap-add.
  return out.OverlapAdd(output_buffer_);
}