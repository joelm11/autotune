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
    // Each FFT bin maps to a fixed frequency band on a linear, constant
    // axis, so a bin's x position always shows which frequency it came from.
    const float step = (float)w / (float)data_.size();
    const float gap = 2.0f;  // horizontal spacing between bars
    const float bar_w = std::max(1.0f, step - gap);

    // Paint spectral levels as bars.
    g.setColour(juce::Colours::limegreen);
    for (size_t i = 0; i < data_.size(); ++i) {
      const float x = (float)i * step + gap * 0.5f;
      const float bar_h = (data_[i] / max) * (float)h;
      g.fillRect(x, (float)h - bar_h, bar_w, bar_h);
    }

    // Paint the calculated fundamental frequency in the top right.
    const unsigned f0 = state_.f0.load();
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    const juce::String label =
        (f0 > 0) ? (juce::String(f0) + " Hz") : juce::String("--");
    g.drawText(label, w - 80, 4, 76, 16, juce::Justification::right, false);
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