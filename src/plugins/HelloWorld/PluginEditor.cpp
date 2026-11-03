#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HelloWorldEditor::HelloWorldEditor(HelloWorldProcessor& p)
    : AudioProcessorEditor(&p)
{
    label.setText("Hello, world!", juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);

    setSize(400, 300);
}

//==============================================================================
void HelloWorldEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void HelloWorldEditor::resized()
{
    label.setBounds(getLocalBounds());
}