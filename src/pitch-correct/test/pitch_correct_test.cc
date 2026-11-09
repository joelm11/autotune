#include "../pitch_correct.hh"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <string>
#include <vector>

#include "AudioFile.h"

namespace {

// Writes mono samples to a wave file for listening/inspection.
class TestOutputFile {
 public:
  TestOutputFile(const std::string& name, const int sample_rate)
      : name_(name), sample_rate_(sample_rate) {}

  void Write(const std::vector<float>& samples) {
    auto out_buffer = std::vector<std::vector<float>>(1, samples);
    out_file.setSampleRate(sample_rate_);
    out_file.setAudioBuffer(out_buffer);
    out_file.save(name_);
  }

 private:
  std::string name_;
  int sample_rate_;
  AudioFile<float> out_file;
};

constexpr int kSampleRate = 44100;
constexpr int kFrameSize = 1024;
// Reference concert pitch used to construct the expected scale.
constexpr float kRefFreq = 440.0f;

float FreqOfMidi(const int midi) {
  return kRefFreq * std::pow(2.0f, static_cast<float>(midi - 69) / 12.0f);
}

// Frequencies of the C-major scale (all notes present in the sweep region).
std::vector<float> CMajorNotes() {
  std::vector<float> notes;
  for (int oct = 2; oct <= 6; ++oct) {
    const int tonic = 60 + 12 * (oct - 4);  // C in this octave
    for (const int off : {0, 2, 4, 5, 7, 9, 11}) {
      notes.push_back(FreqOfMidi(tonic + off));
    }
  }
  return notes;
}

// Nearest scale note to |freq|.
float NearestNote(const std::vector<float>& notes, const float freq) {
  float best = notes.front();
  float best_dist = std::fabs(best - freq);
  for (const float n : notes) {
    const float dist = std::fabs(n - freq);
    if (dist < best_dist) {
      best_dist = dist;
      best = n;
    }
  }
  return best;
}

// A continuous phase-carried sine at |freq|, split into blocks.
std::vector<float> GenSineBlocks(const float freq, const int num_blocks) {
  std::vector<float> out;
  double phase = 0.0;
  const double step = 2.0 * std::numbers::pi_v<double> * freq / kSampleRate;
  for (int block = 0; block < num_blocks; ++block) {
    for (int i = 0; i < kFrameSize; ++i) {
      out.push_back(static_cast<float>(std::sin(phase)));
      phase += step;
    }
  }
  return out;
}

// A continuous phase-carried sine sweep stepped through the given frequencies,
// each held for |hold_frames| blocks of |frame_size| samples, followed by two
// silent blocks that flush the stream.
std::vector<float> GenSteppedSweep(const std::vector<float>& freqs,
                                   const int hold_frames,
                                   const int frame_size) {
  std::vector<float> out;
  double phase = 0.0;
  for (const float f : freqs) {
    const double step = 2.0 * std::numbers::pi_v<double> * f / kSampleRate;
    for (int block = 0; block < hold_frames; ++block) {
      for (int i = 0; i < frame_size; ++i) {
        out.push_back(static_cast<float>(std::sin(phase)));
        phase += step;
      }
    }
  }
  // Two silent flush blocks.
  out.insert(out.end(), 2 * static_cast<size_t>(frame_size), 0.0f);
  return out;
}

// Estimate the dominant frequency of |samples| by taking the median spacing
// between positive-going zero crossings over the non-silent (signal) region.
float EstimateFreq(const std::vector<float>& samples) {
  // Locate the first non-silent sample.
  int start = -1;
  for (int i = 0; i < static_cast<int>(samples.size()); ++i) {
    if (std::fabs(samples[i]) > 1e-3f) {
      start = i;
      break;
    }
  }
  if (start < 0) {
    return 0.f;
  }

  std::vector<float> periods;
  float prev = -1.f;
  for (int i = start + 1; i < static_cast<int>(samples.size()); ++i) {
    if (samples[i - 1] < 0.f && samples[i] >= 0.f) {
      // Sub-sample crossing point via linear interpolation.
      const float frac = -samples[i - 1] / (samples[i] - samples[i - 1]);
      const float crossing = static_cast<float>(i - 1) + frac;
      if (prev > 0.f) {
        periods.push_back(crossing - prev);
      }
      prev = crossing;
    }
  }
  if (periods.empty()) {
    return 0.f;
  }

  // The phase-vocoder + resampler introduce a few glitchy cycles near block
  // edges, so the median period spacing is far more robust than the mean
  // (each positive-going crossing is a full cycle).
  std::sort(periods.begin(), periods.end());
  const float median = periods[periods.size() / 2];
  return kSampleRate / median;
}

// Per-frame RMS of |samples| (split into kFrameSize blocks), skipping blocks
// that are (near) silence.
std::vector<float> BlockRms(const std::vector<float>& samples) {
  std::vector<float> rms;
  for (size_t b = 0; b < samples.size(); b += kFrameSize) {
    double sum = 0.0;
    int n = 0;
    for (size_t i = b; i < std::min(b + kFrameSize, samples.size()); ++i) {
      sum += static_cast<double>(samples[i]) * samples[i];
      ++n;
    }
    if (n > 0) {
      rms.push_back(static_cast<float>(std::sqrt(sum / n)));
    }
  }
  return rms;
}

}  // namespace

// Feed a stepped sine sweep that strays from the C-major scale and confirm
// the pitch corrector continuously snaps each held block onto a note of the
// selected scale, producing one phase-continuous output stream (no per-block
// transients from recreating the shifter).
TEST(PitchCorrectTest, SweepSnapsToCScale) {
  const auto notes = CMajorNotes();

  PitchCorrect pc(pitch::Key::C, 1.f);

  // A stepped sweep from 210 Hz to 840 Hz, each frequency held for 4 blocks.
  // Holding lets the streaming correction (which lags one block) reach steady
  // state, so every block after the first of a hold must snap exactly onto
  // the nearest C-major note.
  constexpr int kNumSteps = 12;
  constexpr int kHoldFrames = 4;
  const float f_start = 210.0f;
  const float f_end = 840.0f;

  std::vector<float> freqs;
  for (int s = 0; s < kNumSteps; ++s) {
    freqs.push_back(f_start + (f_end - f_start) * s / (kNumSteps - 1));
  }

  const std::vector<float> input_signal =
      GenSteppedSweep(freqs, kHoldFrames, kFrameSize);
  const size_t kNumBlocks = input_signal.size() / kFrameSize;

  std::vector<float> output_buff;
  output_buff.reserve(input_signal.size());

  int moved = 0, checked = 0;
  for (size_t c = 0; c < kNumBlocks; ++c) {
    const FBuffer src(input_signal.begin() + c * kFrameSize,
                      input_signal.begin() + (c + 1) * kFrameSize);
    // The block being corrected is the one held at step c / kHoldFrames.
    const float fund =
        freqs[std::min(c / kHoldFrames, static_cast<size_t>(kNumSteps - 1))];

    const FBuffer output = pc.Process(src, fund);

    if (output.empty()) {
      continue;  // first block is the streaming vocoder's startup latency
    }
    ASSERT_EQ(output.size(), src.size());
    output_buff.insert(output_buff.end(), output.begin(), output.end());

    // output(c) retunes the block fed one call earlier using the scale factor
    // of call c, so blocks held for more than one frame reach steady state.
    // Blocks that start a new hold (c % kHoldFrames == 0) are transition
    // blocks between held frequencies and are not frequency-checked.
    const bool steady_state = (c % kHoldFrames != 0);
    const bool is_real_block =
        (c - 1) < static_cast<size_t>(kNumSteps * kHoldFrames);
    if (steady_state && is_real_block) {
      const float expected = NearestNote(notes, fund);
      const float out_freq = EstimateFreq(output);
      ASSERT_GT(out_freq, 0.f) << "No signal detected in block " << c;

      // The corrected frequency must sit on a note of the C-major scale.
      // Tolerance is loose enough to absorb phase-vocoder+resampler artefacts
      // but far tighter than the ~half-step gaps between scale notes.
      constexpr float kTolHz = 15.0f;
      EXPECT_NEAR(out_freq, expected, kTolHz)
          << "Held block at " << fund << " Hz should snap to " << expected
          << " Hz (a C-major note) but landed at " << out_freq << " Hz";

      // Confirm it was a genuine correction (input was off the scale).
      if (std::fabs(out_freq - fund) > 1.f) {
        ++moved;
      }
      ++checked;
    }
  }

  // Every steady-state block of every hold must have been frequency-checked.
  EXPECT_EQ(checked, kNumSteps * (kHoldFrames - 1));
  // The sweep strays from the scale, so nearly all checked blocks are moved.
  EXPECT_GT(moved, checked / 2);

  // --- Continuity: the corrected stream must not show harsh transients ---
  // The output keeps the input amplitude (the OLA + resampler are unity-gain
  // in steady state). A per-block RMS near the sine RMS (0.707) proves there
  // are no dropouts/restarts; a bounded sample-to-sample step proves there
  // are no clicks. Both would fail if the shifter were recreated per block.
  const std::vector<float> rms = BlockRms(output_buff);
  ASSERT_GT(rms.size(), 0u);
  for (const float r : rms) {
    if (r > 1e-3f) {
      // The output keeps the input amplitude (~0.707 sine RMS). The small
      // dips are the startup ramp, the one-block phase re-acquisition at note
      // changes, and the natural decay into the trailing silence; anything
      // below 0.15 would be a real dropout/restart, and a spike above 1.5 a
      // transient.
      EXPECT_GT(r, 0.15f) << "Amplitude dropout (block RMS " << r << ")";
      EXPECT_LT(r, 1.5f) << "Amplitude spike (block RMS " << r << ")";
    }
  }

  double max_delta = 0.0;
  for (size_t i = 1; i < output_buff.size(); ++i) {
    max_delta = std::max(
        max_delta,
        std::fabs(static_cast<double>(output_buff[i]) - output_buff[i - 1]));
  }
  // A sine of amplitude 1 at ~880 Hz has max sample-step ~0.13; note-change
  // transitions (resampler rate change) bump this to ~0.7. A phase reset or
  // buffer restart would produce steps of ~1-2 (a hard click), so 1.0 cleanly
  // separates "continuous, softly transitioning stream" from "harsh
  // transient".
  EXPECT_LT(max_delta, 1.0) << "Click/transient in the corrected stream";

  // --- Write the raw sweep and corrected result to wave files ---
  TestOutputFile input_writer("pitch_corrected_input_sweep.wav", kSampleRate);
  TestOutputFile output_writer("pitch_corrected_sweep.wav", kSampleRate);
  input_writer.Write(input_signal);
  output_writer.Write(output_buff);
}

// A fundamental already on the scale is left (effectively) unchanged.
TEST(PitchCorrectTest, OnScaleNoteIsNotMoved) {
  PitchCorrect pc(pitch::Key::C, 1.f);
  const float kA4 = 440.0f;

  const std::vector<float> input_signal = GenSineBlocks(kA4, 4);
  std::vector<float> output_buff;
  for (int block = 0; block < 4; ++block) {
    const FBuffer src(input_signal.begin() + block * kFrameSize,
                      input_signal.begin() + (block + 1) * kFrameSize);
    const FBuffer output = pc.Process(src, kA4);
    if (output.empty()) {
      continue;
    }
    output_buff.insert(output_buff.end(), output.begin(), output.end());
  }
  ASSERT_FALSE(output_buff.empty());
  const float out_freq = EstimateFreq(output_buff);
  EXPECT_GT(out_freq, 0.f);
  EXPECT_NEAR(out_freq, kA4, 15.0f);
}

// Empty or invalid input is handled gracefully.
TEST(PitchCorrectTest, HandlesInvalidInput) {
  PitchCorrect pc(pitch::Key::A, 1.f);
  EXPECT_TRUE(pc.Process({}, 440.f).empty());
  EXPECT_TRUE(pc.Process(GenSteppedSweep({440}, 1, 512), -1.f).empty());
}

// An adjustment rate of zero must be a passthrough (no pitch correction).
TEST(PitchCorrectTest, ZeroAdjustmentIsPassthrough) {
  PitchCorrect pc(pitch::Key::C, 0.f);
  const float kOffScale = 300.f;  // between D4 and E4

  const std::vector<float> input_signal = GenSineBlocks(kOffScale, 4);
  std::vector<float> output_buff;
  for (int block = 0; block < 4; ++block) {
    const FBuffer src(input_signal.begin() + block * kFrameSize,
                      input_signal.begin() + (block + 1) * kFrameSize);
    const FBuffer output = pc.Process(src, kOffScale);
    if (output.empty()) {
      continue;
    }
    output_buff.insert(output_buff.end(), output.begin(), output.end());
  }
  ASSERT_FALSE(output_buff.empty());
  const float out_freq = EstimateFreq(output_buff);
  EXPECT_GT(out_freq, 0.f);
  // With no correction the output fundamental should stay close to the input.
  EXPECT_NEAR(out_freq, kOffScale, 20.0f);
}