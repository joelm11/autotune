#ifndef VOCODER_IMPL_H
#define VOCODER_IMPL_H

#include <complex>
#include <vector>

#include "fft/fft_impl.h"
#include "ola_buffer.h"
#include "utils.hh"

class VocoderImpl {
 public:
  using SizeType = std::size_t;
  using FBuffer = std::vector<float>;
  using CFBuffer = std::vector<std::complex<float>>;

  VocoderImpl(const SizeType frame_sz, const SizeType sample_rate,
              const float scale_factor);

  FBuffer Process(const FBuffer& src);

 private:
  void Analysis(VocoderImpl::FBuffer& src, VocoderImpl::CFBuffer& out);
  void ModifyPhaseT(VocoderImpl::CFBuffer& an_frame,
                    VocoderImpl::FBuffer& prev_frame_phase,
                    VocoderImpl::FBuffer& prev_frame_synth_phase);
  VocoderImpl::FBuffer Synthesis(VocoderImpl::CFBuffer& src,
                                 OLABuffer<float>& out);

  // Constants.
  const SizeType kAnalysisHops = 4;
  const SizeType kFrameSz_, kSampleRate_;
  const SizeType kAnalysisHopSize, kSynthesisHopSize_;
  const std::vector<float> kWindowBuffer_;

  RingBuffer<float> input_acc_;
  FBuffer prev_frame_phase_, prev_frame_synth_phase_;
  OLABuffer<float> output_acc_;
  std::unique_ptr<FFTImpl> fft_;
};

#endif  // VOCODER_IMPL_H