#ifndef PLUGIN_COMPONENTS_DROPDOWN_H
#define PLUGIN_COMPONENTS_DROPDOWN_H
#pragma once
#include <JuceHeader.h>

#include <functional>
#include <vector>

#include "../fonts.hh"
#include "../palette.hh"
#include "../state.hh"

namespace dropdown_style {
inline constexpr float kFontHeight = 20.0f;
inline constexpr int kFontStyle = (int)juce::Font::bold;

// The label/chevron shares of the usable width.
inline constexpr float kTextFlex = 6.0f;
inline constexpr float kChevronFlex = 1.0f;

inline constexpr float kSidePadding = 8.0f;
inline constexpr float kChevronSize = 8.0f;
inline constexpr float kChevronStroke = 2.0f;
}  // namespace dropdown_style

class Dropdown : public juce::Component {
 public:
  Dropdown() = default;
  explicit Dropdown(State& state) : state_(&state) {}

  void setState(State& state) { state_ = &state; }

  void setOptions(const std::vector<juce::String>& options) {
    options_ = options;
    selected_ = 0;
    repaint();
  }

  void setSelectedIndex(int index) {
    if (index >= 0 && index < (int)options_.size()) {
      selected_ = index;
      repaint();
    }
  }

  int getSelectedIndex() const { return selected_; }

  juce::String getSelectedText() const {
    return options_.empty() ? juce::String() : options_[selected_];
  }

  void setOnChange(std::function<void(int)> callback) {
    on_change_ = std::move(callback);
  }

  void paint(juce::Graphics& g) override {
    const juce::Rectangle<int> bounds = getLocalBounds();

    g.setColour(palette::kSurface0);
    g.fillRect(bounds);
    g.setColour(palette::kSurface2);
    g.drawRect(bounds, 1.0f);

    // Split the usable width between the label and the chevron.
    const float usable =
        (float)bounds.getWidth() - 2.0f * dropdown_style::kSidePadding;
    const float textWidth =
        usable * dropdown_style::kTextFlex /
        (dropdown_style::kTextFlex + dropdown_style::kChevronFlex);

    const juce::Rectangle<float> textArea(
        (float)bounds.getX() + dropdown_style::kSidePadding,
        (float)bounds.getY(), textWidth, (float)bounds.getHeight());
    const juce::Rectangle<float> chevronArea(
        textArea.getRight(), (float)bounds.getY(), usable - textWidth,
        (float)bounds.getHeight());

    g.setColour(palette::kText);
    g.setFont(
        fonts::mono(dropdown_style::kFontHeight, dropdown_style::kFontStyle));
    g.drawText(getSelectedText(), textArea.toNearestInt(),
               juce::Justification::centredLeft);

    const float size = dropdown_style::kChevronSize;
    const float centreX = chevronArea.getCentreX();
    const float centreY = chevronArea.getCentreY();
    juce::Path arrow;
    arrow.startNewSubPath(centreX - size * 0.5f, centreY - size * 0.25f);
    arrow.lineTo(centreX, centreY + size * 0.25f);
    arrow.lineTo(centreX + size * 0.5f, centreY - size * 0.25f);
    g.setColour(palette::kSubtext1);
    g.strokePath(arrow, juce::PathStrokeType(dropdown_style::kChevronStroke));
  }

  void resized() override {}

  void mouseDown(const juce::MouseEvent&) override {
    if (options_.empty()) return;

    juce::PopupMenu menu;
    for (size_t i = 0; i < options_.size(); ++i)
      menu.addItem((int)i + 1, options_[i], true, (int)i == selected_);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
                       [this](int result) {
                         if (result > 0 && result - 1 != selected_) {
                           selected_ = result - 1;
                           if (state_)
                             state_->key = static_cast<pitch::Key>(selected_);
                           if (on_change_) on_change_(selected_);
                           repaint();
                         }
                       });
  }

 private:
  std::vector<juce::String> options_;
  int selected_ = 0;
  State* state_ = nullptr;
  std::function<void(int)> on_change_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Dropdown)
};

#endif  // PLUGIN_COMPONENTS_DROPDOWN_H
