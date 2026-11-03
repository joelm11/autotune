# Define the JUCE plugin build helpers (juce_plugin). Must appear before the
# targets that use them.
include("${CMAKE_SOURCE_DIR}/cmake/plugin.cmake")

# Copy built plugins straight into the system/user plugin folder so DAWs can
# see them without a manual install step. JUCE already defaults the VST3
# destination to "~/Library/Audio/Plug-Ins/VST3" on macOS; turning this flag on
# at directory scope (the properties are INHERITED and read during
# juce_add_plugin) makes every plugin here get copied after building.
set_directory_properties(PROPERTIES
    JUCE_COPY_PLUGIN_AFTER_BUILD TRUE)

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

# Link the pitch-detection library so the plugin can use YIN.
target_link_libraries(HelloWorld PRIVATE pitch-detect)