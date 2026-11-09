#ifndef OLA_BUFFER_H
#define OLA_BUFFER_H

#include <vector>

#include "../api.hh"

template <typename T>
class OLABuffer {
 public:
  OLABuffer(const SizeType frame_size, const SizeType hop_size)
      : kFrameSize_(frame_size),
        hop_size_(hop_size),
        l_(0),
        buffer_(kFrameSize_ + hop_size, 0) {}

  std::vector<T> OverlapAdd(const std::vector<T>& samples) {
    if (samples.size() != kFrameSize_) {
      throw std::runtime_error("Samples size must be equal to window size");
    }

    // Overlap add the samples to the ring buffer.
    for (SizeType i = 0; i < kFrameSize_; ++i) {
      buffer_[(l_ + i) % buffer_.size()] += samples[i];
    }

    // Copy the output frame from the buffer.
    // Zero the shifted out portion.
    std::vector<T> output(hop_size_);
    for (SizeType i = 0; i < hop_size_; ++i) {
      output[i] = buffer_[(l_ + i) % buffer_.size()];
      buffer_[(l_ + i) % buffer_.size()] = 0;
    }

    // Update write pointer
    l_ = (l_ + hop_size_) % buffer_.size();

    return output;
  }

  // Change the synthesis hop size (used to retune the pitch shifter while
  // streaming)
  void SetHopSize(const SizeType hop_size) {
    hop_size_ = hop_size;
    if (buffer_.size() < kFrameSize_ + hop_size) {
      buffer_.resize(kFrameSize_ + hop_size, 0);
    }
  }

 private:
  const SizeType kFrameSize_;

  SizeType hop_size_, l_;
  std::vector<T> buffer_;
};
#endif  // OLA_BUFFER_H