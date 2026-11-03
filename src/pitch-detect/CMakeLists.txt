add_library(pitch-detect STATIC pitch_detect.hh pitch_detect.cc)
target_include_directories(pitch-detect PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


if(STANDALONE_BUILD AND DEBUG)
    unit_test(test_pitch_detect pitch_detect_test.cc "pitch-detect;AudioFile")
endif()