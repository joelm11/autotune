#ifndef PLUGIN_COMPONENTS_DROPDOWN_H
#define PLUGIN_COMPONENTS_DROPDOWN_H
#pragma once
#include <JuceHeader.h>

#include <functional>
#include <vector>

class Dropdown : public juce::Component {
 public:
  Dropdown() = default;

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
    g.fillAll(juce::Colours::white);

    const juce::Rectangle<int> bounds = getLocalBounds();

    g.setColour(juce::Colours::black);
    g.drawRect(bounds, 1.0f);

    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    const juce::Rectangle<int> textArea = bounds.reduced(6, 0);
    g.drawText(getSelectedText(), textArea, juce::Justification::centredLeft);

    // Chevron pointing down, signalling that this opens a menu.
    const float size = 8.0f;
    const float centreX = (float)bounds.getRight() - 14.0f;
    const float centreY = (float)bounds.getCentreY();
    juce::Path arrow;
    arrow.startNewSubPath(centreX - size * 0.5f, centreY - size * 0.25f);
    arrow.lineTo(centreX, centreY + size * 0.25f);
    arrow.lineTo(centreX + size * 0.5f, centreY - size * 0.25f);
    g.setColour(juce::Colours::black);
    g.strokePath(arrow, juce::PathStrokeType(2.0f));
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
                           if (on_change_) on_change_(selected_);
                           repaint();
                         }
                       });
  }

 private:
  std::vector<juce::String> options_;
  int selected_ = 0;
  std::function<void(int)> on_change_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Dropdown)
};

#endif  // PLUGIN_COMPONENTS_DROPDOWN_H
