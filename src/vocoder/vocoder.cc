#include "vocoder.h"

#include <iostream>
Vocoder::Vocoder(const SizeType num_channels, const SizeType num_samples,
                 const SizeType sample_rate)
    : kNumChannels_(num_channels),
      kNumSamples_(num_samples),
      kSampleRate_(sample_rate),
      synthesis_hop_size_(kAnalysisHopSize) {
  InitBuffers(num_channels, num_samples, synthesis_hop_size_);
  InitFFT(num_channels, num_samples);
}

/** TODO: Currently the input is advancing by frame_size samples, when it should
 * be advancing by analysis_hop_size samples. I still want to be able to feed it
 * one frame at a time though.
 */
std::vector<std::vector<float>> Vocoder::Process(
    const std::vector<std::vector<float>>& src, const Effect effect,
    const float scale_factor) {
  ValidateAndUpdate(src, effect, scale_factor);

  Analysis();

  switch (curr_effect_) {
    case Effect::kRobotize:
      ModifyPhaseR();
      break;
    case Effect::kTimeStretch:
      ModifyPhaseT();
      break;
    case Effect::kNone:
    default:
      break;
  }

  return Synthesis();
}

void Vocoder::ValidateAndUpdate(const std::vector<std::vector<float>>& src,
                                const Effect effect, const float scale_factor) {
  // If the channel count changes, throw an error.
  if (src.size() != kNumChannels_) {
    throw std::runtime_error("Vocoder: Channel count mismatch.");
  }
  // If the scale factor changes, update synthesis hop size.
  if (scale_factor != scale_factor_) {
    scale_factor_ = scale_factor;
    synthesis_hop_size_ = kAnalysisHopSize * scale_factor_;
    for (auto& olabuffer : olabuffers_) {
      olabuffer.SetHopSize(synthesis_hop_size_);
    }
  }
  // If the sample count changed, pad with zeros.
  if (src[0].size() < kNumSamples_) {
    for (int i = 0; i < kNumChannels_; ++i) {
      std::copy(src[i].begin(), src[i].end(), input_buffer_[i].begin());
      std::fill(input_buffer_[i].begin() + src[i].size(),
                input_buffer_[i].end(), 0.0f);
    }
  } else {
    input_buffer_ = std::move(src);
  }
  curr_effect_ = effect;
}

void Vocoder::Analysis() {
  // Apply window function.
  for (int i = 0; i < kNumChannels_; ++i) {
    for (int j = 0; j < kNumSamples_; ++j) {
      input_buffer_[i][j] *= kWindowBuffer_[j];
    }
  }
  // FFT
  for (int i = 0; i < kNumChannels_; ++i) {
    ffts_[i]->Forward(input_buffer_[i], fft_output_buffer_[i]);
  }
  // Convert FFT result to magnitude and phase format.
  for (int i = 0; i < kNumChannels_; ++i) {
    for (int j = 0; j < kNumSamples_; ++j) {
      fft_output_buffer_[i][j] =
          std::complex<float>(std::abs(fft_output_buffer_[i][j]),
                              std::arg(fft_output_buffer_[i][j]));
    }
  }
}

void Vocoder::ModifyPhaseR() {
  for (int i = 0; i < kNumChannels_; ++i) {
    for (int j = 0; j < fft_output_buffer_.size(); ++j) {
      fft_output_buffer_[i][j].imag(0);
    }
  }
}

void Vocoder::ModifyPhaseT() {
  std::vector<float> bin_freqs(kNumSamples_), phase_advance(kNumSamples_);
  FBuffer inter_result(kNumChannels_, std::vector<float>(kNumSamples_));
  // Constants.
  for (int j = 0; j < kNumSamples_; ++j) {
    bin_freqs[j] = 2 * M_PI * j / kNumSamples_;
    phase_advance[j] = bin_freqs[j] * kAnalysisHopSize;
  }
  // Wrapped phase advance.
  for (int i = 0; i < kNumChannels_; ++i) {
    for (int j = 0; j < kNumSamples_; ++j) {
      inter_result[i][j] = fft_output_buffer_[i][j].imag() - prev_phase_[i][j] -
                           phase_advance[j];
      inter_result[i][j] = inter_result[i][j] -
                           2 * M_PI * round(inter_result[i][j] / (2 * M_PI));
      prev_phase_[i][j] = fft_output_buffer_[i][j].imag();
    }
  }
  // Instantaneous frequency.
  for (int i = 0; i < kNumChannels_; ++i) {
    for (int j = 0; j < kNumSamples_; ++j) {
      inter_result[i][j] = bin_freqs[j] + inter_result[i][j] / kAnalysisHopSize;
    }
  }
  // Phase propagation and synthesis.
  for (int i = 0; i < kNumChannels_; ++i) {
    for (int j = 0; j < kNumSamples_; ++j) {
      inter_result[i][j] =
          prev_synth_phase_[i][j] + synthesis_hop_size_ * inter_result[i][j];
      fft_output_buffer_[i][j].imag(inter_result[i][j]);
      prev_synth_phase_[i][j] = inter_result[i][j];
    }
  }
}

Vocoder::FBuffer Vocoder::Synthesis() {
  // Convert magnitude and phase back to complex format.
  for (int i = 0; i < kNumChannels_; ++i) {
    for (int j = 0; j < kNumSamples_; ++j) {
      fft_output_buffer_[i][j] = std::polar(fft_output_buffer_[i][j].real(),
                                            fft_output_buffer_[i][j].imag());
    }
  }
  // Inverse FFT.
  for (int i = 0; i < kNumChannels_; ++i) {
    ffts_[i]->Inverse(fft_output_buffer_[i], output_buffer_[i]);
  }
  // Apply window function.
  for (int i = 0; i < kNumChannels_; ++i) {
    for (int j = 0; j < kNumSamples_; ++j) {
      output_buffer_[i][j] *= kWindowBuffer_[j] / kNumSamples_;
    }
  }
  // Overlap-add.
  FBuffer output(kNumChannels_, std::vector<float>(synthesis_hop_size_));
  for (int i = 0; i < kNumChannels_; ++i) {
    output[i] = olabuffers_[i].OverlapAdd(output_buffer_[i]);
  }
  return output;
}

void Vocoder::InitBuffers(const SizeType num_channels,
                          const SizeType num_samples,
                          const SizeType synthesis_hop_size) {
  // Initialize internal buffers.
  input_buffer_ = FBuffer(num_channels, std::vector<float>(num_samples));
  output_buffer_ = FBuffer(num_channels, std::vector<float>(num_samples));
  prev_phase_ = FBuffer(num_channels, std::vector<float>(num_samples));
  prev_synth_phase_ = FBuffer(num_channels, std::vector<float>(num_samples));
  fft_input_buffer_ = FBuffer(num_channels, std::vector<float>(num_samples));
  fft_output_buffer_ =
      CFBuffer(num_channels, std::vector<std::complex<float>>(num_samples));
  olabuffers_ = std::vector<OLABuffer<float>>(
      num_channels, OLABuffer<float>(num_samples, synthesis_hop_size));
};

void Vocoder::InitFFT(const SizeType num_channels, const SizeType num_samples) {
  ffts_.clear();
  for (int i = 0; i < num_channels; ++i) {
    ffts_.push_back(std::make_unique<FFTImpl>(num_samples));
  }
}