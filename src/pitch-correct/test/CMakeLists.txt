# Note that we link against the AudioFile library here to dump the corrected
# audio into a wave file so it can be listened to and inspected.
unit_test(test_pitch_correct pitch_correct_test.cc "pitch-correct;AudioFile")
unit_test(test_smoothing smoothing_test.cc "pitch-correct")