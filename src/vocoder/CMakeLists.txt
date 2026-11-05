add_library(vocoder STATIC vocoder.hh vocoder.cc vocoder_impl.cc)
target_include_directories(vocoder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(vocoder PUBLIC fft_impl utils)

if(STANDALONE_BUILD AND DEBUG)
    add_subdirectory(test)
endif()