#ifndef VOCODER_IMPL_H
#define VOCODER_IMPL_H
#include "../../api.hh"
#include "../utils/fft/fft_impl.h"
#include "../utils/ola_buffer.h"
#include "utils.hh"

class VocoderImpl {
 public:
  VocoderImpl(const SizeType frame_sz, const SizeType sample_rate,
              const float scale_factor);

  void SetScaleFactor(const float scale_factor);

  FBuffer Process(const FBuffer& src);

 private:
  void Analysis(FBuffer& src, CFBuffer& out);
  void ModifyPhaseT(CFBuffer& an_frame, FBuffer& prev_frame_phase,
                    FBuffer& prev_frame_synth_phase,
                    std::vector<bool>& prev_frame_peaks);
  FBuffer Synthesis(CFBuffer& src, OLABuffer<float>& out);

  // Locate the spectral peaks of an analysis frame: strict local maxima of
  // the magnitude spectrum on the positive-frequency half (bins 1..N/2-1),
  // keeping only peaks above kPeakThresholdRatio of the frame's largest
  // peak. Bins 0 and N/2 are not candidates and keep their propagated phase.
  std::vector<SizeType> FindPeaks(const CFBuffer& an_frame) const;

  const SizeType kAnalysisHops = 4;
  const float kPeakThresholdRatio = 1e-3f;
  const SizeType kFrameSz_, kSampleRate_;
  const SizeType kAnalysisHopSize;
  SizeType kSynthesisHopSize_;
  const FBuffer kAnalysisWindow_;
  FBuffer kSynthesisWindow_;

  // Phase locking (Laroche & Dolson 1999): bins in the influence zone of a
  // spectral peak keep their phase locked to that peak, so the wave shape of
  // each partial is preserved while the peak's own phase carries the time
  // scaling. The two quantities below hold the per-bin phase state that must
  // survive across frames: the raw analysis phase of the previous frame and
  // the accumulated synthesis phase. prev_frame_peaks_ records which bins
  // were peaks in the previous analysis frame so that newly appeared peaks
  // can be given identity phase locking (which preserves transients).
  RingBuffer<float> input_acc_;
  FBuffer prev_frame_phase_, prev_frame_synth_phase_;
  std::vector<bool> prev_frame_peaks_;
  OLABuffer<float> output_acc_;
  std::unique_ptr<FFTImpl> fft_;
};

#endif  // VOCODER_IMPL_H