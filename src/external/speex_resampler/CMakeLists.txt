add_library(speex_resampler speex_resampler.h speex_resampler.c)
target_include_directories(speex_resampler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


if(STANDALONE_BUILD AND DEBUG)
    unit_test(speex_resampler_test test/speex_resampler_test.cc speex_resampler)
endif()