#include "../api.hh"
#include "../external/speex_resampler/speex_resampler.h"
#include "resampler_base.h"

class Resampler : public ResamplerBase {
 public:
  Resampler(const int in_rate, const int out_rate, const int num_samples)
      : in_rate_(in_rate),
        out_rate_(out_rate),
        num_samples_in_(num_samples),
        num_samples_out_(num_samples * (float)out_rate / in_rate),
        resampler_(speex_resampler_init(1, in_rate, out_rate, 10, 0)),
        output_buffer_(FBuffer(num_samples_out_)) {
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
    output_buffer_.resize(out_samples);
  }

  // Set the resampling ratio output_per_input = ratio_num / ratio_den so the
  // filter honours the true duration ratio (speex args are swapped for this).
  void SetRatio(const unsigned int ratio_num, const unsigned int ratio_den) {
    speex_resampler_set_rate_frac(resampler_, ratio_den, ratio_num, in_rate_,
                                  out_rate_);
  }

  unsigned int NumSamplesOut() const { return num_samples_out_; }

  FBuffer Resample(const FBuffer& input) override {
    uint32_t in = num_samples_in_, out = num_samples_out_;
    speex_resampler_process_float(resampler_, 0, input.data(), &in,
                                  output_buffer_.data(), &out);
    return output_buffer_;
  }

 private:
  unsigned in_rate_, out_rate_;
  unsigned num_samples_in_, num_samples_out_;
  SpeexResamplerState* resampler_;
  FBuffer output_buffer_;
};