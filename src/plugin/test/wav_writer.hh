#pragma once

#include <string>
#include <vector>

#include "AudioFile.h"

namespace plugin_test {

// Writes `samples` (mono, in roughly [-1, 1]) as a 16-bit WAV at `sample_rate`.
// Returns false if the file could not be created or written.
inline bool writeWavFile(const std::string& path, int sample_rate,
                         const std::vector<float>& samples) {
  AudioFile<float> file;
  file.setSampleRate(static_cast<uint32_t>(sample_rate));
  file.setBitDepth(16);
  file.setAudioBuffer({samples});
  return file.save(path);
}

// Loads a mono version (first channel) of the WAV at `path` into `samples`,
// and reports its sample rate. Returns false if the file could not be read.
inline bool readWavFile(const std::string& path, int& sampleRateOut,
                        std::vector<float>& samplesOut) {
  AudioFile<float> file;
  if (!file.load(path)) return false;

  sampleRateOut = static_cast<int>(file.getSampleRate());
  const auto& firstChannel = file.samples[0];
  samplesOut.assign(firstChannel.begin(), firstChannel.end());
  return true;
}

}  // namespace plugin_test