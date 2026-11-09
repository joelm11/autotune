include("${CMAKE_SOURCE_DIR}/cmake/plugin.cmake")
set_directory_properties(PROPERTIES JUCE_COPY_PLUGIN_AFTER_BUILD TRUE)

juce_plugin(June
    COMPANY_NAME  "Jabratory"
    PRODUCT_NAME  "June"
    PLUGIN_NAME   "June"
    VERSION       "0.1.0"
    SOURCES
        editor.hh
        editor.cc
        processor_base.hh
        processor.hh
        processor.cc
        )

target_link_libraries(June PRIVATE pitch-detect)
target_link_libraries(June PRIVATE pitch-correct)
target_link_libraries(June PRIVATE readerwriterqueue)

if(STANDALONE_BUILD AND DEBUG)
    add_subdirectory(test)
endif()