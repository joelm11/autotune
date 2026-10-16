#include "../vocoder.hh"

#include <gtest/gtest.h>

#include <numbers>

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
    auto out = voc.Process(
        std::vector<float>(sine.begin() + samples_procd,
                           sine.begin() + samples_procd + chunk_sz));
    output_buff.insert(output_buff.end(), out.begin(), out.end());
  }

  // Writeout file and validate fs write
  fout.Write(output_buff);
}

// Stretch speech with the vocoder

// Compress speech with the vocoder
