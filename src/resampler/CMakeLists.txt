add_library(resampler INTERFACE resampler.h)
target_link_libraries(resampler INTERFACE speex_resampler)
target_include_directories(resampler INTERFACE "${CMAKE_SOURCE_DIR}/src/external/speex_resampler" ${CMAKE_CURRENT_SOURCE_DIR})

if (STANDALONE_BUILD AND DEBUG)
    unit_test(resampler_test test/resampler_test.cc resampler)
endif()