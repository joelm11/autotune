#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Barebones editor. Extend/override this class for real UI work.
class HelloWorldEditor  : public juce::AudioProcessorEditor
{
public:
    explicit HelloWorldEditor(HelloWorldProcessor&);
    ~HelloWorldEditor() override = default;

    //==========================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelloWorldEditor)
};