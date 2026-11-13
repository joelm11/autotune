#ifndef PLUGIN_COMPONENTS_SPECTRAL_H
#define PLUGIN_COMPONENTS_SPECTRAL_H
#pragma once
#include <JuceHeader.h>

#include <algorithm>
#include <cmath>

#include "../state.hh"

class Spectral : public juce::Component, public juce::Timer {
 public:
  explicit Spectral(State& state) : state_(state) { startTimerHz(15); }

  void paint(juce::Graphics& g) override {
    g.fillAll(juce::Colours::darkgrey);
    if (data_.size() < 2) return;

    const int w = getWidth(), h = getHeight();
    // Average adjacent FFT bin pairs so only half the bins are shown. This
    // keeps the frequency axis linear while reducing visual granularity.
    const size_t bins = data_.size() / 2;
    const float step = (float)w / (float)bins;
    const float gap = 2.0f;  // horizontal spacing between bars
    const float bar_w = std::max(1.0f, step - gap);

    // Use a fixed full-scale reference (a full-amplitude sine sits at 0 dB)
    // so the displayed magnitude stays constant from frame to frame instead
    // of being rescaled to each frame's own peak. Only levels inside
    // [kMinDb, kMaxDb] are rendered.
    const float ref = (float)data_.size();
    const float range = kMaxDb - kMinDb;

    // Paint spectral levels as bars.
    g.setColour(juce::Colours::limegreen);
    for (size_t i = 0; i < bins; ++i) {
      const float mag = (data_[2 * i] + data_[2 * i + 1]) * 0.5f;
      if (mag <= 0.0f) continue;

      const float db = 20.0f * std::log10(mag / ref);
      const float level = (db - kMinDb) / range;
      if (level <= 0.0f) continue;

      const float x = (float)i * step + gap * 0.5f;
      const float bar_h = std::min(1.0f, level) * (float)h;
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

  // Rendering window, in dB relative to a full-scale sine (0 dBFS). A
  // 60 dB range is a typical spectrum-analyzer scale for a compact display.
  static constexpr float kMinDb = -60.0f;
  static constexpr float kMaxDb = 0.0f;

  State& state_;
  FBuffer data_;
};

#endif  // PLUGIN_COMPONENTS_SPECTRAL_H