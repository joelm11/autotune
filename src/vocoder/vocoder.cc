#include "vocoder.hh"

#include <memory>

#include "vocoder_impl.hh"

Vocoder::Vocoder(const SizeType frame_size, const SizeType sample_rate,
                 const SizeType scale_factor)
    : impl_(std::make_unique<VocoderImpl>(frame_size, sample_rate,
                                          scale_factor)) {}

Vocoder::~Vocoder() {}

Vocoder::FBuffer Vocoder::Process(const FBuffer& src) {
  return impl_->Process(src);
}