#include "../vocoder.hh"

#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

#include "../window.h"
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
