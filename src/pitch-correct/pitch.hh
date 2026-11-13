#ifndef PITCH_H
#define PITCH_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace pitch {

enum class Key { C, G, D, A, E };

inline constexpr size_t kNumKeys = 5;

constexpr float kRefFreq = 440.0f;  // A4
constexpr int kRefMidi = 69;        // A4
constexpr float kCMidi = 60.0f;     // C4
constexpr int kMajorScale[7] = {0, 2, 4, 5, 7, 9, 11};
constexpr int kKeyRoot[kNumKeys] = {0, 7, 2, 9, 4};

inline double MidiOfFreq(const float freq) {
  return kRefMidi + 12.0 * std::log2(static_cast<double>(freq) / kRefFreq);
}

inline float FreqOfMidi(const int midi) {
  return kRefFreq * std::pow(2.0f, static_cast<float>(midi - kRefMidi) / 12.0f);
}

// Octave whose tonic sits at or just below |fund_freq|.
inline unsigned OctaveOf(const float fund_freq, const Key key) {
  const double midi = MidiOfFreq(fund_freq);
  const int fund_oct = 4 + static_cast<int>(std::floor((midi - kCMidi) / 12.0));

  int fund_pc = static_cast<int>(std::floor(std::fmod(midi, 12.0) + 0.5)) % 12;
  if (fund_pc < 0) {
    fund_pc += 12;
  }

  const int tonic_pc = kKeyRoot[static_cast<size_t>(key)];
  const int octave = (tonic_pc <= fund_pc) ? fund_oct : fund_oct - 1;
  return static_cast<unsigned>(std::max(0, octave));
}

inline std::vector<float> ScaleNoteFreqs(const unsigned octave, const Key key) {
  const int tonic = static_cast<int>(kCMidi) +
                    12 * (static_cast<int>(octave) - 4) +
                    kKeyRoot[static_cast<size_t>(key)];

  std::vector<float> freqs;
  freqs.reserve(7);
  for (const int offset : kMajorScale) {
    freqs.push_back(FreqOfMidi(tonic + offset));
  }
  return freqs;
}

// Nearest scale note to |fund_freq|.
inline float NearestNoteFreq(const float fund_freq, const Key key) {
  const unsigned center = OctaveOf(fund_freq, key);

  float best = 0.f;
  float best_dist = std::numeric_limits<float>::max();
  for (int d = -1; d <= 1; ++d) {
    const int octave = static_cast<int>(center) + d;
    if (octave < 0) {
      continue;
    }
    for (const float note :
         ScaleNoteFreqs(static_cast<unsigned>(octave), key)) {
      const float dist = std::fabs(note - fund_freq);
      if (dist < best_dist) {
        best_dist = dist;
        best = note;
      }
    }
  }
  return best;
}

}  // namespace pitch
#endif  // PITCH_H
