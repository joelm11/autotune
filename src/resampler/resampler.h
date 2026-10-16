#include <vector>

#include "resampler_base.h"
#include "speex_resampler.h"

class Resampler : public ResamplerBase {
 public:
  Resampler(const int in_rate, const int out_rate, const int num_channels,
            const int num_samples)
      : kNumChannels_(num_channels),
        in_rate_(in_rate),
        out_rate_(out_rate),
        num_samples_in_(num_samples),
        num_samples_out_(num_samples * (float)out_rate / in_rate),
        resampler_(
            speex_resampler_init(num_channels, in_rate, out_rate, 10, 0)),
        output_buffer_(num_channels, std::vector<float>(num_samples_out_)) {
    speex_resampler_skip_zeros(resampler_);
  }

  ~Resampler() { speex_resampler_destroy(resampler_); }

  void SetRate(const int in_rate, const int out_rate) {
    if (out_rate_ != out_rate) {
      in_rate_ = in_rate;
      out_rate_ = out_rate;
      speex_resampler_set_rate(resampler_, in_rate, out_rate);
      num_samples_out_ = num_samples_in_ * (float)out_rate / in_rate;
    }
  }

  void SetInOutSamples(const int in_samples, const int out_samples) {
    num_samples_in_ = in_samples;
    num_samples_out_ = out_samples;
    output_buffer_.resize(kNumChannels_, std::vector<float>(out_samples));
  }

  std::vector<std::vector<float>> Resample(
      const std::vector<std::vector<float>>& input) override {
    // Iterate over input channels and use resamplers to resample each channel.
    for (int i = 0; i < kNumChannels_; ++i) {
      uint32_t in = num_samples_in_, out = num_samples_out_;
      speex_resampler_process_float(resampler_, i, input[i].data(), &in,
                                    output_buffer_[i].data(), &out);
    }
    return output_buffer_;
  }

 private:
  const unsigned kNumChannels_;
  unsigned in_rate_, out_rate_;
  unsigned num_samples_in_, num_samples_out_;
  SpeexResamplerState* resampler_;
  std::vector<std::vector<float>> output_buffer_;
};