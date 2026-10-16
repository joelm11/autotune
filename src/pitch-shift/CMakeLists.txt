add_library(pitch-shift STATIC pitch_shift.hh pitch_shift.cc)
target_include_directories(pitch-shift PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


if(STANDALONE_BUILD AND DEBUG)
    add_subdirectory(test)
endif()