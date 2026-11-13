#include "../vocoder.hh"

#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

#include "../../utils/window.hh"
#include "AudioFile.h"

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

std::vector<float> GenSine(const int sr, const float f, const int num_samples) {
  std::vector<float> out(num_samples);
  for (int i = 0; i < num_samples; ++i) {
    out[i] = std::sin(2 * std::numbers::pi * i * f / sr);
  }
  return out;
}

// A sawtooth built from its first num_harmonics harmonics (amplitude 1/h). It
// has a strongly peaked, phase-coherent waveform: all partials share the same
// phase relationship, which is exactly the structure phase locking must
// preserve under time scaling.
std::vector<float> GenSawtooth(const int sr, const float f, const int num_harmonics,
                               const int num_samples) {
  std::vector<float> out(num_samples, 0.0f);
  for (int h = 1; h <= num_harmonics; ++h) {
    const float amp = 1.0f / h;
    for (int i = 0; i < num_samples; ++i) {
      out[i] += amp * std::sin(2 * std::numbers::pi * h * f * i / sr);
    }
  }
  return out;
}

// Highest normalized cross-correlation of x against y (y taken at the integer
// shift d), for shifts in [lo, hi]. Used to check whether an output waveform
// matches a reference regardless of the vocoder's latency/phase offset.
double BestCorr(const std::vector<float>& x, const std::vector<float>& y,
                const int lo, const int hi) {
  double ex2 = 0.0;
  for (double v : x) ex2 += v * v;
  double best = -2.0;
  for (int d = lo; d <= hi; ++d) {
    double num = 0.0, ey2 = 0.0;
    for (int i = 0; i < static_cast<int>(x.size()); ++i) {
      const int j = i + d;
      if (j < 0 || j >= static_cast<int>(y.size())) continue;
      num += static_cast<double>(x[i]) * y[j];
      ey2 += static_cast<double>(y[j]) * y[j];
    }
    if (ey2 <= 0.0) continue;
    best = std::max(best, num / std::sqrt(ex2 * ey2));
  }
  return best;
}

class VocoderTest : public testing::Test {
 protected:
  const int kFrameSize = 1024;
  const int kSampleRate = 16e3;
  AudioFile<float> writer;
};

// Run the vocoder on a sinewave with no stretching and output to a file for
// visual inspection
TEST_F(VocoderTest, UniformSine) {
  TestOutputFile fout("uniform_sine.wav", kSampleRate);

  Vocoder voc(kFrameSize, kSampleRate, 1.f);

  auto sine = GenSine(kSampleRate, 440, 32e3);

  std::vector<float> output_buff;
  for (int samples_procd = 0; samples_procd < sine.size();
       samples_procd += kFrameSize) {
    const int chunk_sz =
        std::min(kFrameSize, static_cast<int>(sine.size()) - samples_procd);
    auto out = voc.Process(std::vector<float>(
        sine.begin() + samples_procd, sine.begin() + samples_procd + chunk_sz));
    output_buff.insert(output_buff.end(), out.begin(), out.end());
  }

  // Writeout file and validate fs write
  fout.Write(output_buff);
}

// The vocoder applies its window once during analysis and once during
// synthesis, so each frame is effectively weighted by window^2. Verify that
// ComputeHannWin normalizes the window for its hop so the overlap-add of
// window^2 is exactly 1 for every sample (i.e. the window is COLA-correct and
// the output has a flat envelope with unity gain).
TEST_F(VocoderTest, WindowCola) {
  const std::vector<SizeType> hops = {128, 256, 512};
  for (const SizeType hop : hops) {
    const auto window = ComputeHannWin<float>(kFrameSize, hop);

    // Check the overlap-add sum at every sample in the steady-state middle of
    // a long frame stream.
    double max_dev = 0.0;
    for (int n = 4 * kFrameSize; n < 8 * kFrameSize; ++n) {
      double sum = 0.0;
      for (long m = -static_cast<long>(kFrameSize / hop) - 1;
           m <= n / static_cast<long>(hop); ++m) {
        const long idx = n - m * static_cast<long>(hop);
        if (idx < 0 || idx >= kFrameSize) {
          continue;
        }
        const double v = window[idx];
        sum += v * v;
      }
      max_dev = std::max(max_dev, std::fabs(sum - 1.0));
    }
    EXPECT_NEAR(max_dev, 0.0, 1e-6) << "hop = " << hop;
  }
}

// Stretch speech with the vocoder.
TEST_F(VocoderTest, StretchSpeech) {
  AudioFile<float> input_file;
  ASSERT_TRUE(input_file.load(AUDIO_FILE_PATH));

  // The dialogue clip is stereo, so take its first channel as the mono source.
  const std::vector<float>& speech = input_file.samples[0];
  const int output_rate = static_cast<int>(input_file.getSampleRate());
  const float kScaleFactor = 2.f;

  TestOutputFile fout("vocoded_audio_s.wav", output_rate);

  Vocoder voc(kFrameSize, output_rate, kScaleFactor);

  std::vector<float> output_buff;
  for (int samples_procd = 0; samples_procd < speech.size();
       samples_procd += kFrameSize) {
    const int chunk_sz =
        std::min(kFrameSize, static_cast<int>(speech.size()) - samples_procd);
    auto out = voc.Process(
        std::vector<float>(speech.begin() + samples_procd,
                           speech.begin() + samples_procd + chunk_sz));
    output_buff.insert(output_buff.end(), out.begin(), out.end());
  }

  // Stretching should produce more samples than it consumed.
  EXPECT_GT(output_buff.size(), speech.size());
  // Output duration should be roughly scale_factor x the input duration.
  EXPECT_NEAR(static_cast<double>(output_buff.size()) / speech.size(),
              kScaleFactor, 0.1);

  // Write out file and validate fs write
  fout.Write(output_buff);
}

// Phase locking (Laroche & Dolson 1999) ties the bins of each spectral peak's
// influence zone to the peak's propagated phase, keeping the partial's wave
// shape intact under time scaling. Unlocked per-bin phase accumulation makes
// the partials drift apart instead, smearing the waveform into "phasy" mush.
//
// A sawtooth is the ideal probe: its harmonics are all phase-locked to each
// other in the source, so stretching must yield a waveform that still matches
// the (same-pitch) sawtooth reference. The plain vocoder collapses here
// (correlation ~0.5 at 2x stretch) while the locked version stays ~0.98+.
TEST_F(VocoderTest, PhaseLockingPreservesWaveformShape) {
  constexpr float kF0 = 110.0f;
  constexpr int kNumHarmonics = 48;
  constexpr int kNumSamples = 16e3;  // 1 s

  const auto saw = GenSawtooth(kSampleRate, kF0, kNumHarmonics, kNumSamples);
  const auto ref = GenSawtooth(kSampleRate, kF0, kNumHarmonics, kNumSamples);

  for (const float scale : {2.0f, 0.5f}) {
    Vocoder voc(kFrameSize, kSampleRate, scale);

    std::vector<float> output_buff;
    for (int samples_procd = 0; samples_procd < saw.size();
         samples_procd += kFrameSize) {
      const int chunk_sz = std::min(
          kFrameSize, static_cast<int>(saw.size()) - samples_procd);
      auto out = voc.Process(std::vector<float>(
          saw.begin() + samples_procd, saw.begin() + samples_procd + chunk_sz));
      output_buff.insert(output_buff.end(), out.begin(), out.end());
    }

    // Correlate a steady-state slice of the output against the same slice of
    // the reference sawtooth. The maximum over a +/- one frame shift absorbs
    // the vocoder's latency and absolute phase offset.
    const int x_beg = static_cast<int>(output_buff.size()) / 4;
    const int x_end = static_cast<int>(output_buff.size()) / 2;
    const int ref_beg = kNumSamples / 8;
    std::vector<float> x(output_buff.begin() + x_beg,
                         output_buff.begin() + x_end);
    std::vector<float> y(ref.begin() + ref_beg,
                         ref.begin() + ref_beg + (x_end - x_beg));
    const double corr = BestCorr(x, y, -kFrameSize, kFrameSize);

    // With phase locking the partials stay mutually coherent, so the output
    // tracks the reference closely at any stretch factor (measured ~0.99 for
    // both cases; the unlocked vocoder scores ~0.53 at 2x and ~0.92 at 0.5x).
    EXPECT_GT(corr, scale > 1.f ? 0.80 : 0.95)
        << "scale_factor = " << scale;
  }
}

// Compress speech with the vocoder.
TEST_F(VocoderTest, CompressSpeech) {
  AudioFile<float> input_file;
  ASSERT_TRUE(input_file.load(AUDIO_FILE_PATH));

  // The dialogue clip is stereo, so take its first channel as the mono source.
  const std::vector<float>& speech = input_file.samples[0];
  const int output_rate = static_cast<int>(input_file.getSampleRate());
  const float kScaleFactor = 0.5f;

  TestOutputFile fout("vocoded_audio_c.wav", output_rate);

  Vocoder voc(kFrameSize, output_rate, kScaleFactor);

  std::vector<float> output_buff;
  for (int samples_procd = 0; samples_procd < speech.size();
       samples_procd += kFrameSize) {
    const int chunk_sz =
        std::min(kFrameSize, static_cast<int>(speech.size()) - samples_procd);
    auto out = voc.Process(
        std::vector<float>(speech.begin() + samples_procd,
                           speech.begin() + samples_procd + chunk_sz));
    output_buff.insert(output_buff.end(), out.begin(), out.end());
  }

  // Compressing should produce fewer samples than it consumed.
  EXPECT_LT(output_buff.size(), speech.size());
  // Output duration should be roughly scale_factor x the input duration.
  EXPECT_NEAR(static_cast<double>(output_buff.size()) / speech.size(),
              kScaleFactor, 0.1);

  // Write out file and validate fs write
  fout.Write(output_buff);
}
