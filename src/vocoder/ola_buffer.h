#ifndef OLA_BUFFER_H
#define OLA_BUFFER_H

#include <cstddef>
#include <vector>

template <typename T>
class OLABuffer {
 public:
  using Size = std::size_t;

  OLABuffer(const Size frame_size, const Size hop_size)
      : kFrameSize_(frame_size),
        kBufferSize_(kFrameSize_ + hop_size),
        hop_size_(hop_size),
        l_(0),
        buffer_(kBufferSize_, 0) {}

  std::vector<T> OverlapAdd(const std::vector<T>& samples) {
    if (samples.size() != kFrameSize_) {
      throw std::runtime_error("Samples size must be equal to window size");
    }

    // Overlap add the samples to the ring buffer.
    for (Size i = 0; i < kFrameSize_; ++i) {
      buffer_[(l_ + i) % kBufferSize_] += samples[i];
    }

    // Copy the output frame from the buffer.
    // Zero the shifted out portion.
    std::vector<T> output(hop_size_);
    for (std::size_t i = 0; i < hop_size_; ++i) {
      output[i] = buffer_[(l_ + i) % kBufferSize_];
      buffer_[(l_ + i) % kBufferSize_] = 0;
    }

    // Update write pointer
    l_ = (l_ + hop_size_) % kBufferSize_;

    return output;
  }

  void SetHopSize(const Size hop_size) { hop_size_ = hop_size; }

 private:
  const Size kFrameSize_, kBufferSize_;

  Size hop_size_, l_;
  std::vector<T> buffer_;
};
#endif