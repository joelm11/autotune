# Plugin unit test.
#
# This exercises the full JUCE AudioProcessor end to end, so it pulls in the
# JUCE modules. Output WAV files are written via the AudioFile helper (the same
# one the DSP tests use) rather than JUCE's WavAudioFormat.
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

    # The dialogue test reads the same mono input the vocoder tests use.
    target_compile_definitions(test_plugin PRIVATE
        AUDIO_FILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../vocoder/test/resources/dialog_clip_stereo.wav")

    target_link_libraries(test_plugin PRIVATE
        juce::juce_audio_utils
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
        pitch-detect
        pitch-shift
        readerwriterqueue
        AudioFile
        GTest::gtest_main)

    # processor_base.hh references the plugin-name macros that juce_add_plugin
    # normally injects; provide them for the plain console-app test target.
    target_compile_definitions(test_plugin PRIVATE JucePlugin_Name="June")

    # Disable JUCE's injected `using namespace juce;` so AudioFile.h's global
    # WavAudioFormat enum and JUCE's juce::WavAudioFormat don't collide when
    # AudioFile's templates are instantiated in the same TU. The test sources
    # qualify every JUCE name explicitly, so this is safe.
    target_compile_definitions(test_plugin PRIVATE
        DONT_SET_USING_JUCE_NAMESPACE)

    # Generate a JuceHeader.h for this target from the linked JUCE modules.
    juce_generate_juce_header(test_plugin)

    gtest_discover_tests(test_plugin DISCOVERY_MODE PRE_TEST)
endif()