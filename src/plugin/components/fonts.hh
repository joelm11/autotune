#ifndef PLUGIN_FONTS_H
#define PLUGIN_FONTS_H
#pragma once
#include <JuceHeader.h>

namespace fonts {

//! The concrete monospaced family used across the UI. Change this to any
//! installed font, e.g. "Monaco", "Menlo", "Courier New".
inline const juce::String& family() {
  static const juce::String name = "Monaco";
  return name;
}

//! A monospaced font from an explicitly named family.
inline juce::Font mono(const juce::String& family, float height,
                       int style = juce::Font::plain) {
  return juce::Font(juce::FontOptions(family, height, style));
}

//! The UI monospaced font at the given height and style. Naming the family
//! explicitly guarantees monospaced glyphs even if another LookAndFeel takes
//! over as the application default.
inline juce::Font mono(float height, int style = juce::Font::plain) {
  return mono(family(), height, style);
}

}  // namespace fonts

#endif  // PLUGIN_FONTS_H
