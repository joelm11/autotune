add_definitions(-DAUDIO_FILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/dialog_clip_stereo.wav")

# Note that we link against the AudioFile library here to dump samples into wave files and inspect them.
unit_test(test_vocoder vocoder_test.cc "vocoder;AudioFile")
unit_test(test_ola_buffer ola_buffer_test.cc "AudioFile")