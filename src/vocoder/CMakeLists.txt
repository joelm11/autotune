add_library(vocoder STATIC vocoder.h vocoder.cc window.h ola_buffer.h)
target_include_directories(vocoder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(fft)
target_link_libraries(vocoder PUBLIC fft_impl)

if(STANDALONE_BUILD AND DEBUG)
    add_subdirectory(test)
endif()