#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HelloWorldProcessor::HelloWorldProcessor()
     : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

HelloWorldProcessor::~HelloWorldProcessor()
{
}

//==============================================================================
const juce::String HelloWorldProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HelloWorldProcessor::acceptsMidi() const
{
    return false;
}

bool HelloWorldProcessor::producesMidi() const
{
    return false;
}

bool HelloWorldProcessor::isMidiEffect() const
{
    return false;
}

double HelloWorldProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HelloWorldProcessor::getNumPrograms()
{
    return 1;
}

int HelloWorldProcessor::getCurrentProgram()
{
    return 0;
}

void HelloWorldProcessor::setCurrentProgram(int index)
{
}

const juce::String HelloWorldProcessor::getProgramName(int index)
{
    return {};
}

void HelloWorldProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void HelloWorldProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Reset processing state here if needed.
}

void HelloWorldProcessor::releaseResources()
{
    // Release any resources used during playback.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HelloWorldProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
            || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono());
}
#endif

void HelloWorldProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& midiMessages)
{
    // "Hello world": pass the input through untouched.
}

//==============================================================================
bool HelloWorldProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* HelloWorldProcessor::createEditor()
{
    return new HelloWorldEditor(*this);
}

//==============================================================================
void HelloWorldProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
}

void HelloWorldProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from the memory block.
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HelloWorldProcessor();
}