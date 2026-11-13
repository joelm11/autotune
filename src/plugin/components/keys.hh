#ifndef PLUGIN_COMPONENTS_KEYS_H
#define PLUGIN_COMPONENTS_KEYS_H
#include <JuceHeader.h>

#include <set>
#include <vector>

#include "../palette.hh"

class Keys : public juce::Component {
 public:
  Keys() = default;

  //! Sets the lowest note of the rendered octave (default: 60 = C4).
  void setBaseNote(int midiNote) {
    base_ = midiNote;
    repaint();
  }

  //! Highlights every key in the octave that contains one of the given MIDI
  //! notes. Pass an empty vector to clear all highlights.
  void setHighlightedNotes(const std::vector<int>& midiNotes) {
    highlighted_.clear();
    highlighted_.insert(midiNotes.begin(), midiNotes.end());
    repaint();
  }

  void paint(juce::Graphics& g) override {
    g.fillAll(palette::kMantle);

    const float w = (float)getWidth();
    const float h = (float)getHeight();

    constexpr int kWhitePitch[7] = {0, 2, 4, 5, 7, 9, 11};
    constexpr int kBlackPitch[5] = {1, 3, 6, 8, 10};
    // Index of the white key immediately to the left of each black key.
    constexpr int kBlackAfter[5] = {0, 1, 3, 4, 5};

    const float whiteW = w / 7.0f;

    // White keys.
    for (int i = 0; i < 7; ++i) {
      const juce::Rectangle<float> key((float)i * whiteW, 0.0f, whiteW, h);
      g.setColour(isHighlighted(kWhitePitch[i]) ? palette::kMauve
                                                : palette::kText);
      g.fillRect(key);
      g.setColour(palette::kSurface2);
      g.drawRect(key, 1.0f);
    }

    // Black keys, centred on the boundaries between the white keys.
    const float blackW = whiteW * 0.6f;
    const float blackH = h * 0.62f;
    for (int i = 0; i < 5; ++i) {
      const float centre = ((float)kBlackAfter[i] + 0.5f) * whiteW;
      const juce::Rectangle<float> key(centre - blackW * 0.5f, 0.0f, blackW,
                                       blackH);
      g.setColour(isHighlighted(kBlackPitch[i]) ? palette::kMauve
                                                : palette::kCrust);
      g.fillRect(key);
    }

    g.setColour(palette::kSurface2);
    g.drawRect(getLocalBounds(), 1.0f);
  }

  void resized() override {}

 private:
  //! True if the octave (base_..base_+11) holds a highlighted note with the
  //! given pitch class.
  bool isHighlighted(int pitchClass) const {
    for (int note : highlighted_) {
      const int rel = note - base_;
      if (rel >= 0 && rel < 12 && rel % 12 == pitchClass) return true;
    }
    return false;
  }

  std::set<int> highlighted_;
  int base_ = 60;  // C4

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Keys)
};

#endif  // PLUGIN_COMPONENTS_KEYS_H
