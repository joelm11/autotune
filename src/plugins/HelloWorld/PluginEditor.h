#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class HelloWorldEditor  : public juce::AudioProcessorEditor
{
public:
    explicit HelloWorldEditor(HelloWorldProcessor&);

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::Label label;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelloWorldEditor)
};