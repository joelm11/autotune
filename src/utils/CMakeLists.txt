add_library(utils INTERFACE window.hh ola_buffer.h)

target_include_directories(utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(fft)

if(STANDALONE_BUILD AND DEBUG)
    add_subdirectory(test)
endif()