#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HelloWorldEditor::HelloWorldEditor(HelloWorldProcessor& p)
    : AudioProcessorEditor(&p)
{
    setSize(400, 300);
}

//==============================================================================
void HelloWorldEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void HelloWorldEditor::resized()
{
    // No child components in the barebones version.
}