# Plugin unit test.
#
# Unlike the DSP tests (which dump output via the AudioFile helper), this one
# exercises the full JUCE AudioProcessor end to end, so it pulls in the JUCE
# modules and uses JUCE's WavAudioFormat to write the offline output file.
if(STANDALONE_BUILD AND DEBUG)
    juce_add_console_app(test_plugin VERSION "0.1.0")

    target_sources(test_plugin PRIVATE
        processor_test.cc
        ../editor.cc
        ../processor.cc)

    # Allow the test sources to include processor.hh etc. and the local helpers.
    target_include_directories(test_plugin PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..)

    target_link_libraries(test_plugin PRIVATE
        juce::juce_audio_utils
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
        pitch-detect
        pitch-shift
        readerwriterqueue
        GTest::gtest_main)

    # processor_base.hh references the plugin-name macros that juce_add_plugin
    # normally injects; provide them for the plain console-app test target.
    target_compile_definitions(test_plugin PRIVATE JucePlugin_Name="June")

    # Generate a JuceHeader.h for this target from the linked JUCE modules.
    juce_generate_juce_header(test_plugin)

    gtest_discover_tests(test_plugin DISCOVERY_MODE PRE_TEST)
endif()