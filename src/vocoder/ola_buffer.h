#pragma once

#include <cstddef>
#include <vector>

template <typename T>
class OLABuffer {
 public:
  OLABuffer(const std::size_t frame_size, const std::size_t hop_size)
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
    for (std::size_t i = 0; i < kFrameSize_; ++i) {
      buffer_[(l_ + i) % kBufferSize_] += samples[i];
    }

    // Copy the output frame from the buffer.
    std::vector<T> output(hop_size_);
    for (std::size_t i = 0; i < hop_size_; ++i) {
      output[i] = buffer_[(l_ + i) % kBufferSize_];
    }

    // Zero out the portion of the buffer collected for output and update the
    // write pointer.
    for (std::size_t i = 0; i < hop_size_; ++i) {
      buffer_[(l_ + i) % kBufferSize_] = 0;
    }
    l_ = (l_ + hop_size_) % kBufferSize_;

    return output;
  }

  void SetHopSize(const std::size_t hop_size) { hop_size_ = hop_size; }

 private:
  const std::size_t kFrameSize_, kBufferSize_;
  std::size_t hop_size_, l_;
  std::vector<T> buffer_;
};