#ifndef API_RESAMPLER_RESAMPLER_BASE_H_
#define API_RESAMPLER_RESAMPLER_BASE_H_
#include <vector>

class ResamplerBase {
 public:
  virtual ~ResamplerBase() = default;

  virtual std::vector<float> Resample(const std::vector<float>& input) = 0;
};

#endif