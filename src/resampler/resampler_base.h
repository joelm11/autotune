#ifndef API_RESAMPLER_RESAMPLER_BASE_H
#define API_RESAMPLER_RESAMPLER_BASE_H
#include "../api.hh"

class ResamplerBase {
 public:
  virtual ~ResamplerBase() = default;

  virtual FBuffer Resample(const FBuffer& input) = 0;
};

#endif  // API_RESAMPLER_RESAMPLER_BASE_H