#ifndef PLUGIN_PALETTE_H
#define PLUGIN_PALETTE_H
#pragma once
#include <JuceHeader.h>

//! Catppuccin Mocha palette (https://catppuccin.com).
//!
//! The named colours mirror the upstream theme so components can reference
//! them directly. The MonoLookAndFeel also maps the relevant ones onto JUCE's
//! standard widget colour IDs, so built-in widgets are themed consistently.
namespace palette {

inline const juce::Colour kRosewater{0xfff5e0dc};
inline const juce::Colour kFlamingo{0xfff2cdcd};
inline const juce::Colour kPink{0xfff5c2e7};
inline const juce::Colour kMauve{0xffcba6f7};
inline const juce::Colour kRed{0xfff38ba8};
inline const juce::Colour kMaroon{0xffeba0ac};
inline const juce::Colour kPeach{0xfffab387};
inline const juce::Colour kYellow{0xfff9e2af};
inline const juce::Colour kGreen{0xffa6e3a1};
inline const juce::Colour kTeal{0xff94e2d5};
inline const juce::Colour kSky{0xff89dceb};
inline const juce::Colour kSapphire{0xff74c7ec};
inline const juce::Colour kBlue{0xff89b4fa};
inline const juce::Colour kLavender{0xffb4befe};

inline const juce::Colour kText{0xffcdd6f4};
inline const juce::Colour kSubtext1{0xffbac2de};
inline const juce::Colour kSubtext0{0xffa6adc8};
inline const juce::Colour kOverlay2{0xff9399b2};
inline const juce::Colour kOverlay1{0xff7f849c};
inline const juce::Colour kOverlay0{0xff6c7086};
inline const juce::Colour kSurface2{0xff585b70};
inline const juce::Colour kSurface1{0xff45475a};
inline const juce::Colour kSurface0{0xff313244};
inline const juce::Colour kBase{0xff1e1e2e};
inline const juce::Colour kMantle{0xff181825};
inline const juce::Colour kCrust{0xff11111b};

}  // namespace palette

#endif  // PLUGIN_PALETTE_H
