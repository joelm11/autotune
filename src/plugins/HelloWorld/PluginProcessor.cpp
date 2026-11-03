#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HelloWorldProcessor::HelloWorldProcessor()
     : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

//==============================================================================
void HelloWorldProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Nothing to prepare in the barebones version.
}

void HelloWorldProcessor::releaseResources()
{
    // Nothing to release in the barebones version.
}

void HelloWorldProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& midi)
{
    // Audio-only pass-through: leave the incoming samples untouched.
    juce::ignoreUnused(midi);
}

//==============================================================================
juce::AudioProcessorEditor* HelloWorldProcessor::createEditor()
{
    return new HelloWorldEditor(*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HelloWorldProcessor();
}