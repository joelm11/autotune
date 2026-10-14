add_library(pitch-shift-api pitch_shift_api.h pitch_shift_api.cc)

add_subdirectory(vocoder)
add_subdirectory(resampler)

target_link_libraries(pitch-shift-api PUBLIC vocoder resampler)
target_include_directories(pitch-shift-api PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if(STANDALONE_BUILD AND DEBUG)
    add_subdirectory(test)
endif()