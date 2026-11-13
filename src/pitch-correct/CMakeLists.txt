add_library(pitch-correct STATIC pitch_correct.hh pitch_correct.cc pitch.hh smoothing.hh)
target_include_directories(pitch-correct PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(pitch-correct PUBLIC pitch-shift)

if(STANDALONE_BUILD AND DEBUG)
    add_subdirectory(test)
endif()