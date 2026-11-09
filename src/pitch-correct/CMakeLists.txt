add_library(pitch-correct STATIC pitch_correct.hh pitch_correct.cc)
target_include_directories(pitch-correct PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
