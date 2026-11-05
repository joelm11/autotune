#ifndef PLUGIN_COMPONENTS_SPECTRAL_H
#define PLUGIN_COMPONENTS_SPECTRAL_H
#pragma once
#include <JuceHeader.h>

#include <algorithm>

#include "../state.hh"

class Spectral : public juce::Component, public juce::Timer {
 public:
  explicit Spectral(State& state) : state_(state) { startTimerHz(30); }

  void paint(juce::Graphics& g) override {
    g.fillAll(juce::Colours::darkgrey);
    if (data_.empty()) return;

    const float max = *std::max_element(data_.begin(), data_.end());
    if (max <= 0.0f) return;

    const int w = getWidth(), h = getHeight();
    const float step = (float)w / (float)data_.size();

    juce::Path path;
    for (size_t i = 0; i < data_.size(); ++i) {
      const float x = (float)i * step;
      const float y = h - (data_[i] / max) * (float)h;
      if (i == 0)
        path.startNewSubPath(x, y);
      else
        path.lineTo(x, y);
    }
    g.setColour(juce::Colours::limegreen);
    g.strokePath(path, juce::PathStrokeType(1.5f));
  }

  void resized() override {}

  void timerCallback() override {
    update();
    repaint();
  }

 private:
  void update() {
    // Drain the queue and keep only the most recent frame so we never
    // display stale samples.
    FBuffer latest;
    while (state_.spectra.try_dequeue(latest)) {
    }
    if (!latest.empty()) data_ = std::move(latest);
  }

  State& state_;
  FBuffer data_;
};

#endif  // PLUGIN_COMPONENTS_SPECTRAL_H