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

// Standard concert pitch (A4 = 440 Hz).
constexpr float kRefFreq = 440.0f;
// MIDI note number of A4.
constexpr int kRefMidi = 69;
// MIDI note number of middle C (C4).
constexpr float kCMidi = 60.0f;
// Semitone offsets of a major scale relative to its tonic.
constexpr int kMajorScale[7] = {0, 2, 4, 5, 7, 9, 11};
// Tonic pitch class (C = 0) of each key, indexed by pitch::Key.
constexpr int kKeyRoot[kNumKeys] = {0, 7, 2, 9, 4};

// MIDI note number (possibly fractional) of |freq| in Hz.
inline double MidiOfFreq(const float freq) {
  return kRefMidi + 12.0 * std::log2(static_cast<double>(freq) / kRefFreq);
}

// Frequency (Hz) of the given integer MIDI note number.
inline float FreqOfMidi(const int midi) {
  return kRefFreq * std::pow(2.0f, static_cast<float>(midi - kRefMidi) / 12.0f);
}

// Returns the octave for |key| such that |fund_freq| falls within the
// one-octave span [tonic, tonic + one octave).
inline unsigned OctaveOf(const float fund_freq, const Key key) {
  const double midi = MidiOfFreq(fund_freq);
  const int fund_oct = 4 + static_cast<int>(std::floor((midi - kCMidi) / 12.0));

  // Pitch class (0..11) of the fundamental.
  int fund_pc = static_cast<int>(std::floor(std::fmod(midi, 12.0) + 0.5)) % 12;
  if (fund_pc < 0) {
    fund_pc += 12;
  }

  // Place the tonic in the octave at or just below the fundamental so the
  // fundamental sits inside [tonic, tonic + an octave).
  const int tonic_pc = kKeyRoot[static_cast<size_t>(key)];
  const int octave = (tonic_pc <= fund_pc) ? fund_oct : fund_oct - 1;
  return static_cast<unsigned>(std::max(0, octave));
}

// Frequencies (Hz) of the |key| major scale rooted at |octave| (7 notes).
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

// Frequency (Hz) of the scale note nearest to |fund_freq|. Searches the
// octave containing the fundamental plus its neighbours so the nearest note is
// found even right on an octave boundary.
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
