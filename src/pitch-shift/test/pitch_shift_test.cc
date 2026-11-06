#include "../pitch_shift.hh"

#include <gtest/gtest.h>

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

class PitchShiftTest : public testing::Test {
 protected:
  const int kFrameSize = 1024;
  AudioFile<float> writer;
};

// Shift the pitch of the dialogue clip up. With a scale factor of 2 the phase
// vocoder stretches the audio to twice its length (keeping the pitch intact)
// and the resampler brings it back to the original duration, leaving the pitch
// an octave higher. The result is written to a file for listening and visual
// inspection.
TEST_F(PitchShiftTest, ShiftDialogueUp) {
  AudioFile<float> input_file;
  ASSERT_TRUE(input_file.load(AUDIO_FILE_PATH));

  // The dialogue clip is stereo, so take its first channel as the mono source.
  const std::vector<float>& speech = input_file.samples[0];
  const int sample_rate = static_cast<int>(input_file.getSampleRate());
  const float kScaleFactor = 2.f;

  TestOutputFile fout("pitch_shifted_up.wav", sample_rate);

  PitchShifter ps(kFrameSize, sample_rate, kScaleFactor);

  std::vector<float> output_buff;
  for (int samples_procd = 0; samples_procd < speech.size();
       samples_procd += kFrameSize) {
    const int chunk_sz =
        std::min(kFrameSize, static_cast<int>(speech.size()) - samples_procd);
    auto out = ps.Process(
        std::vector<float>(speech.begin() + samples_procd,
                           speech.begin() + samples_procd + chunk_sz));
    output_buff.insert(output_buff.end(), out.begin(), out.end());
  }

  // Pitch shifting should preserve the duration of the source.
  EXPECT_GT(output_buff.size(), 0u);
  EXPECT_NEAR(static_cast<double>(output_buff.size()) / speech.size(), 1.0,
              0.1);

  // Write out file and validate fs write
  fout.Write(output_buff);
}

// Every non-empty Process must return the same number of samples it was given,
// at the correct duration-preserving resampling ratio (no truncation/padding).
TEST_F(PitchShiftTest, ReturnsSameSampleCount) {
  // Feed contiguous, vocoder-aligned frames exactly as a streaming caller
  // (and the audio tests) would. Streaming mode makes even the first frame
  // full length, its startup latency appearing as leading zeros.
  const std::vector<int> frame_sizes = {512, 1024, 2048};
  const int kNumFrames = 12;

  for (const float kScaleFactor : {0.5f, 1.5f, 2.f, 3.f}) {
    for (const int frame_size : frame_sizes) {
      PitchShifter ps(frame_size, 44100, kScaleFactor);
      for (int f = 0; f < kNumFrames; ++f) {
        auto out = ps.Process(std::vector<float>(frame_size, 0.5f));
        if (out.empty()) continue;
        EXPECT_EQ(out.size(), static_cast<SizeType>(frame_size));
      }
    }
  }
}

// Shift the pitch of the dialogue clip down. With a scale factor of 0.5 the
// phase vocoder shortens the audio to half its length (keeping the pitch
// intact) and the resampler stretches it back to the original duration,
// leaving the pitch an octave lower. The result is written to a file for
// listening and visual inspection.
TEST_F(PitchShiftTest, ShiftDialogueDown) {
  AudioFile<float> input_file;
  ASSERT_TRUE(input_file.load(AUDIO_FILE_PATH));

  // The dialogue clip is stereo, so take its first channel as the mono source.
  const std::vector<float>& speech = input_file.samples[0];
  const int sample_rate = static_cast<int>(input_file.getSampleRate());
  const float kScaleFactor = 0.5f;

  TestOutputFile fout("pitch_shifted_down.wav", sample_rate);

  PitchShifter ps(kFrameSize, sample_rate, kScaleFactor);

  std::vector<float> output_buff;
  for (int samples_procd = 0; samples_procd < speech.size();
       samples_procd += kFrameSize) {
    const int chunk_sz =
        std::min(kFrameSize, static_cast<int>(speech.size()) - samples_procd);
    auto out = ps.Process(
        std::vector<float>(speech.begin() + samples_procd,
                           speech.begin() + samples_procd + chunk_sz));
    output_buff.insert(output_buff.end(), out.begin(), out.end());
  }

  // Pitch shifting should preserve the duration of the source.
  EXPECT_GT(output_buff.size(), 0u);
  EXPECT_NEAR(static_cast<double>(output_buff.size()) / speech.size(), 1.0,
              0.1);

  // Write out file and validate fs write
  fout.Write(output_buff);
}
