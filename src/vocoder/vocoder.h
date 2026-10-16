#ifndef VOCODER_H_
#define VOCODER_H_

#include <vector>

#include "fft/fft_impl.h"
#include "ola_buffer.h"
#include "window.h"

class Vocoder {
 public:
  using SizeType = std::size_t;
  using FBuffer = std::vector<std::vector<float>>;
  using CFBuffer = std::vector<std::vector<std::complex<float>>>;

  enum Effect {
    kNone = 0,
    kRobotize = 1,
    kTimeStretch = 2,
  };

  Vocoder(SizeType num_channels, SizeType num_samples, SizeType sample_rate);

  // Process the given frame applying effect.
  // As the vocoder uses overlap-add to reconstruct the signal,
  // GetAnalysisHopSize() samples will be available as output from each
  // iteration
  // TODO: Shift out remaining samples
  std::vector<std::vector<float>> Process(
      const std::vector<std::vector<float>>& src, const Effect effect,
      const float scale_factor = 1.f);

  // Number of samples returned by each Process() call
  SizeType GetSynthesisHopSize() const { return synthesis_hop_size_; }
  //  Returns the number of samples expected between consecutive input frames
  SizeType GetAnalysisHopSize() const { return kAnalysisHopSize; }

 private:
  void ValidateAndUpdate(const std::vector<std::vector<float>>& src,
                         const Effect effect, const float scale_factor);
  void Analysis();
  void ModifyPhaseR();
  void ModifyPhaseT();
  FBuffer Synthesis();
  void InitBuffers(const SizeType num_channels, const SizeType num_samples,
                   const SizeType synthesis_hop_size);
  void InitFFT(const SizeType num_channels, const SizeType num_samples);

  const SizeType kNumChannels_;
  const SizeType kNumSamples_;
  const SizeType kSampleRate_;
  const SizeType kAnalysisHopSize = kNumSamples_ / 4;
  const std::vector<float> kWindowBuffer_ =
      ComputeHannWin<float>(kNumSamples_, kAnalysisHopSize);

  FBuffer input_buffer_, output_buffer_, fft_input_buffer_, prev_phase_,
      prev_synth_phase_;
  CFBuffer fft_output_buffer_;
  // NOTE: Using a vector of 1D FFTs for simplicity.
  // TODO: Update to use a multidimensional FFT implementation later.
  std::vector<std::unique_ptr<FFTImpl>> ffts_;
  std::vector<OLABuffer<float>> olabuffers_;
  SizeType synthesis_hop_size_;
  Effect curr_effect_;
  float scale_factor_;
};

#endif  // VOCODER_H_