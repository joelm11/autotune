#ifndef VOCODER_IMPL_H
#define VOCODER_IMPL_H
#include "../../api.hh"
#include "fft/fft_impl.h"
#include "ola_buffer.h"
#include "utils.hh"

class VocoderImpl {
 public:
  VocoderImpl(const SizeType frame_sz, const SizeType sample_rate,
              const float scale_factor);

  FBuffer Process(const FBuffer& src);

 private:
  void Analysis(FBuffer& src, CFBuffer& out);
  void ModifyPhaseT(CFBuffer& an_frame, FBuffer& prev_frame_phase,
                    FBuffer& prev_frame_synth_phase);
  FBuffer Synthesis(CFBuffer& src, OLABuffer<float>& out);

  const SizeType kAnalysisHops = 4;
  const SizeType kFrameSz_, kSampleRate_;
  const SizeType kAnalysisHopSize, kSynthesisHopSize_;
  const FBuffer kAnalysisWindow_;
  const FBuffer kSynthesisWindow_;

  RingBuffer<float> input_acc_;
  FBuffer prev_frame_phase_, prev_frame_synth_phase_;
  OLABuffer<float> output_acc_;
  std::unique_ptr<FFTImpl> fft_;
};

#endif  // VOCODER_IMPL_H