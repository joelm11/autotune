#pragma once

#include <JuceHeader.h>

//==============================================================================
// Barebones audio-only processor. Extend/override this class for real work.
class HelloWorldProcessor  : public juce::AudioProcessor
{
public:
    HelloWorldProcessor();
    ~HelloWorldProcessor() override = default;

    //==========================================================================
    // Audio processing
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;

    //==========================================================================
    // Editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==========================================================================
    // Identity
    const juce::String getName() const override { return JucePlugin_Name; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==========================================================================
    // No MIDI - audio only.
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }

    //==========================================================================
    // Programs - required by JUCE, but unused here.
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return {}; }
    void changeProgramName(int index, const juce::String& newName) override {}

    //==========================================================================
    // Persistence
    void getStateInformation(juce::MemoryBlock& destData) override {}
    void setStateInformation(const void* data, int sizeInBytes) override {}

private:
    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelloWorldProcessor)
};