add_library(fft_impl INTERFACE fft_base.h fft_impl.h)

target_link_libraries(fft_impl INTERFACE kissfft)

if(STANDALONE_BUILD AND DEBUG)
    add_subdirectory(test)
endif()