add_definitions(-DAUDIO_FILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../vocoder/test/resources/dialog_clip_stereo.wav")

# Note that we link against the AudioFile library here to dump samples into wave files and inspect them.
unit_test(test_pitch_shift pitch_shift_test.cc "pitch-shift;vocoder;resampler;AudioFile")