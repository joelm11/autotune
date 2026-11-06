#pragma once

#include <JuceHeader.h>

#include <memory>
#include <string>
#include <vector>

namespace plugin_test {

// Writes `samples` (mono, in roughly [-1, 1]) as a 16-bit WAV at `sample_rate`.
// Returns false if the file could not be created or written.
inline bool writeWavFile(const std::string& path, int sample_rate,
                         const std::vector<float>& samples) {
  // JUCE's File constructor demands an absolute path; evaluate the caller's
  // path against the working directory (harmless if it's already absolute).
  juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile(path));
  file.deleteFile();

  std::unique_ptr<juce::FileOutputStream> rawStream(file.createOutputStream());
  if (rawStream == nullptr) return false;

  // createWriterFor() transfers ownership of the stream to the writer on
  // success, so hand it over as the base OutputStream type it expects.
  std::unique_ptr<juce::OutputStream> stream(std::move(rawStream));

  juce::WavAudioFormat format;
  const auto options = juce::AudioFormatWriterOptions{}
                           .withSampleRate(sample_rate)
                           .withNumChannels(1)
                           .withBitsPerSample(16);
  auto writer = format.createWriterFor(stream, options);
  if (writer == nullptr) return false;

  juce::AudioBuffer<float> buffer(1, static_cast<int>(samples.size()));
  buffer.copyFrom(0, 0, samples.data(), static_cast<int>(samples.size()));

  // The WAV header is finalised when the writer is destroyed.
  return writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
}

}  // namespace plugin_test