# FFT library
set(KISSFFT_TEST OFF CACHE BOOL "Build kissfft test")
set(KISSFFT_TOOLS OFF CACHE BOOL "Build kissfft tools")
add_subdirectory(kissfft)

# WAV file handling library
set(BUILD_EXAMPLES OFF)
set(BUILD_TESTS OFF)
add_subdirectory(AudioFile)

# Resampling implementation
add_subdirectory(speex_resampler)