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

  /**
   * @brief Given a frame of audio samples, process the frame using the vocoder.
   * As the vocoder uses overlap-add to reconstruct the signal,
   * GetAnalysisHopSize() samples will be available as output from each
   * iteration.
   * TODO: Add the ability to shift out the remaining processed samples when
   * the input is finished.
   * @param src
   * @param scale_factor
   * @return std::vector<std::vector<float>>
   */
  std::vector<std::vector<float>> Process(
      const std::vector<std::vector<float>>& src, const Effect effect,
      const float scale_factor = 1.f);

  /**
   * @brief Returns the number of samples that will be available as output from
   * the vocoder. The number of samples returned from the vocoder is
   * proportional to the scale factor, which determines the synthesis hop size.
   *
   * @return SizeType
   */
  SizeType GetSynthesisHopSize() const { return synthesis_hop_size_; }

  /**
   * @brief Returns the number of samples expected between consecutive input
   * frames.
   *
   * @return SizeType
   */
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