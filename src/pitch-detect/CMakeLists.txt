add_library(pitch-detect INTERFACE)
target_include_directories(pitch-detect INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pitch-detect INTERFACE vocoder)


if(STANDALONE_BUILD AND DEBUG)
    unit_test(test_pitch_detect pitch_detect_test.cc "pitch-detect;AudioFile")
endif()