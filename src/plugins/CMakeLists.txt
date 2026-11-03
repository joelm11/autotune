# Define the JUCE plugin build helpers (juce_plugin). Must appear before the
# targets that use them.
include("${CMAKE_SOURCE_DIR}/cmake/plugin.cmake")

juce_plugin(HelloWorld
    COMPANY_NAME  "Autotune"
    PRODUCT_NAME  "HelloWorld"
    PLUGIN_NAME   "Hello World"
    VERSION       "1.0.0"
    SOURCES
        HelloWorld/PluginEditor.h
        HelloWorld/PluginEditor.cpp
        HelloWorld/PluginProcessor.h
        HelloWorld/PluginProcessor.cpp)