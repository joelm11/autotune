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

// Loads a mono version (first channel) of the WAV at `path` into `samples`,
// and reports its sample rate. Returns false if the file could not be read.
inline bool readWavFile(const std::string& path, int& sampleRateOut,
                        std::vector<float>& samplesOut) {
  juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile(path));

  juce::AudioFormatManager manager;
  manager.registerBasicFormats();
  std::unique_ptr<juce::AudioFormatReader> reader(
      manager.createReaderFor(file));
  if (reader == nullptr) return false;

  const auto numSamples = static_cast<int>(reader->lengthInSamples);
  juce::AudioBuffer<float> decoded(static_cast<int>(reader->numChannels),
                                   numSamples);
  if (!reader->read(&decoded, 0, numSamples, 0, true, true)) return false;

  sampleRateOut = static_cast<int>(reader->sampleRate);
  samplesOut.assign(decoded.getReadPointer(0),
                    decoded.getReadPointer(0) + numSamples);
  return true;
}

}  // namespace plugin_test