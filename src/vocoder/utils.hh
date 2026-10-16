#ifndef VOCODER_UTILS_H
#define VOCODER_UTILS_H

#include <vector>

using namespace std;

template <typename T>
class RingBuffer {
 public:
  RingBuffer() {}
  RingBuffer(const int frame_size)
      : frame_size(frame_size), N(frame_size * 2 + 1) {
    buffer = vector<T>(N, 0);
  }

  void Write(const vector<T>& samples) {
    for (int sample = 0; sample < samples.size(); ++sample) {
      buffer[w_idx] = samples[sample];
      w_idx = (w_idx + 1) % N;
    }
  }

  // Read frame_size samples each time, but only advance the read pointer by
  // hop_size
  vector<T> Read(const int hop_size) {
    vector<T> out(frame_size);
    for (int sample = 0; sample < frame_size; ++sample) {
      out[sample] = buffer[(r_idx + sample) % N];
    }
    r_idx = (r_idx + hop_size) % N;

    return out;
  }

  // True once at least 2 full frames have accumulated, and stays true while
  // reading another frame would not run up to the write pointer.
  bool Full() const {
    const int remaining = (w_idx - r_idx + N) % N;
    return remaining > frame_size;
  }

 private:
  int frame_size, N;
  vector<T> buffer;
  int w_idx = 0, r_idx = 0;
};

#endif  // VOCODER_UTILS_H