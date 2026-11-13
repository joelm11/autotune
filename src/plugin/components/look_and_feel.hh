#ifndef PLUGIN_LOOK_AND_FEEL_H
#define PLUGIN_LOOK_AND_FEEL_H
#pragma once
#include <JuceHeader.h>

#include "fonts.hh"
#include "palette.hh"

// Sizing for popup menus, which is where the dropdown renders its option list.
namespace popup_menu_style {

inline constexpr float kFontHeight = 20.0f;
inline constexpr int kFontStyle = (int)juce::Font::plain;

// 0 derives the row height from the font.
inline constexpr float kItemHeight = 0.0f;

}  // namespace popup_menu_style

//! LookAndFeel that renders every font with the configured monospaced family
//! and themes JUCE's standard widgets with the Catppuccin palette.
class MonoLookAndFeel : public juce::LookAndFeel_V4 {
 public:
  MonoLookAndFeel() {
    setDefaultSansSerifTypefaceName(fonts::family());

    setColour(juce::ResizableWindow::backgroundColourId, palette::kBase);

    setColour(juce::Label::textColourId, palette::kText);
    setColour(juce::Label::backgroundColourId, palette::kSurface0);
    setColour(juce::Label::outlineColourId, palette::kSurface2);

    setColour(juce::ComboBox::backgroundColourId, palette::kSurface0);
    setColour(juce::ComboBox::textColourId, palette::kText);
    setColour(juce::ComboBox::outlineColourId, palette::kSurface2);
    setColour(juce::ComboBox::arrowColourId, palette::kSubtext1);
    setColour(juce::ComboBox::buttonColourId, palette::kSurface1);

    setColour(juce::PopupMenu::backgroundColourId, palette::kSurface0);
    setColour(juce::PopupMenu::textColourId, palette::kText);
    setColour(juce::PopupMenu::headerTextColourId, palette::kSubtext1);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, palette::kBlue);
    setColour(juce::PopupMenu::highlightedTextColourId, palette::kBase);

    setColour(juce::TextButton::buttonColourId, palette::kSurface1);
    setColour(juce::TextButton::buttonOnColourId, palette::kBlue);
    setColour(juce::TextButton::textColourOffId, palette::kText);
    setColour(juce::TextButton::textColourOnId, palette::kBase);
  }

  //! Forces *every* font through the configured monospaced family, whatever
  //! typeface name it asks for. Because this LookAndFeel is installed as the
  //! default, it is the single choke point for all typeface resolution.
  juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override {
    if (auto typeface = juce::Typeface::createSystemTypefaceFor(
            juce::Font(juce::FontOptions(fonts::family(), font.getHeight(),
                                         font.getStyleFlags())))) {
      return typeface;
    }
    return juce::LookAndFeel_V4::getTypefaceForFont(font);
  }

  //! The exact font the dropdown renders with. Change the family in
  //! fonts::family() to any installed monospaced font.
  static const juce::String& dropdownFontFamily() { return fonts::family(); }

  static constexpr float kDropdownFontHeight = 16.0f;

  //! Font for the closed dropdown box.
  juce::Font getDropdownFont() const {
    return fonts::mono(dropdownFontFamily(), kDropdownFontHeight,
                       juce::Font::bold);
  }

  // Widget fonts are built from the monospaced family explicitly, so they stay
  // monospaced even if another LookAndFeel takes over as the global default.
  juce::Font getLabelFont(juce::Label& label) override {
    const auto& font = label.getFont();
    return fonts::mono(font.getHeight(), font.getStyleFlags());
  }

  juce::Font getComboBoxFont(juce::ComboBox& box) override {
    return fonts::mono(juce::jmin(16.0f, (float)box.getHeight() * 0.85f));
  }

  //! Size of each popup option row. A live itemHeight() of 0 keeps JUCE's
  //! font-derived height.
  void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
                                 int standardMenuItemHeight, int& idealWidth,
                                 int& idealHeight) override {
    juce::LookAndFeel_V4::getIdealPopupMenuItemSize(
        text, isSeparator, standardMenuItemHeight, idealWidth, idealHeight);
    if (!isSeparator) {
      const int row = juce::roundToInt(popup_menu_style::kItemHeight);
      if (row > 0) idealHeight = row;
    }
  }

  //! The option list is sized to its widest entry, so cap it at the closed
  //! dropdown box (the menu's target component) and let the text ellipsise
  //! rather than the menu spilling outside the control's width.
  void getIdealPopupMenuItemSizeWithOptions(
      const juce::String& text, bool isSeparator, int standardMenuItemHeight,
      int& idealWidth, int& idealHeight,
      const juce::PopupMenu::Options& options) override {
    juce::LookAndFeel_V4::getIdealPopupMenuItemSizeWithOptions(
        text, isSeparator, standardMenuItemHeight, idealWidth, idealHeight,
        options);

    if (auto* target = options.getTargetComponent()) {
      const int border = getPopupMenuBorderSizeWithOptions(options);
      idealWidth =
          juce::jmin(idealWidth, juce::jmax(0, target->getWidth() - border * 2));
    }
  }

  //! Font for the drop-down list. Must be overridden separately from the box
  //! text, because the popup is drawn in its own window.
  juce::Font getPopupMenuFont() override {
    return fonts::mono(dropdownFontFamily(), popup_menu_style::kFontHeight,
                       popup_menu_style::kFontStyle);
  }

  juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override {
    return fonts::mono(juce::jmin(16.0f, (float)buttonHeight * 0.6f));
  }
};

//! Shared instance. It must outlive every component that references it.
inline MonoLookAndFeel& monoLookAndFeel() {
  static MonoLookAndFeel look_and_feel;
  return look_and_feel;
}

#endif  // PLUGIN_LOOK_AND_FEEL_H
