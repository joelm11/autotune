# Helpers for building JUCE-based audio plugins (VST3 + Standalone).
#
# JUCE is pulled in as an external dependency via a git submodule at
#   src/external/JUCE
# and its CMake targets/functions are made available by the
#   add_subdirectory(JUCE)
# call in src/external/CMakeLists.txt.
#
# Configurable options:
#   JUCE_BUILD_STANDALONE - if ON (default), build the runnable standalone
#                           app alongside the VST3 plugin.

option(JUCE_BUILD_STANDALONE "Build the standalone app for JUCE plugins" ON)

# Convenience wrapper around juce_add_plugin() that always targets exactly the
# two formats this project cares about (VST3 and, optionally, Standalone) and
# disables the JUCE features we don't use (web browser / Curl).
#
# Optional arguments:
#   COMPANY_NAME    - vendor string used in plugin metadata.
#   PRODUCT_NAME    - human readable plugin / product name.
#   PLUGIN_NAME     - name shown to the user when loading the plugin in a host.
#   PLUGIN_CODE     - 4 character plugin code (used by VST3).
#   VERSION         - semantic version string (defaults to 1.0.0).
#   SOURCES         - list of implementation files compiled into the plugin.
function(juce_plugin target)
    set(oneValueArgs COMPANY_NAME PRODUCT_NAME PLUGIN_NAME PLUGIN_CODE VERSION)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_COMPANY_NAME)
        set(ARG_COMPANY_NAME "Autotune")
    endif()
    if(NOT ARG_PRODUCT_NAME)
        set(ARG_PRODUCT_NAME ${target})
    endif()
    if(NOT ARG_PLUGIN_NAME)
        set(ARG_PLUGIN_NAME ${ARG_PRODUCT_NAME})
    endif()
    if(NOT ARG_PLUGIN_CODE)
        set(ARG_PLUGIN_CODE "Atun")
    endif()
    if(NOT ARG_VERSION)
        set(ARG_VERSION "1.0.0")
    endif()

    # Only VST3 and Standalone are supported in this project.
    set(_formats VST3)
    if(JUCE_BUILD_STANDALONE)
        list(APPEND _formats Standalone)
    endif()

    juce_add_plugin(${target}
        VERSION ${ARG_VERSION}
        COMPANY_NAME "${ARG_COMPANY_NAME}"
        PRODUCT_NAME "${ARG_PRODUCT_NAME}"
        PLUGIN_NAME "${ARG_PLUGIN_NAME}"
        PLUGIN_MANUFACTURER_CODE "AutT"
        PLUGIN_CODE "${ARG_PLUGIN_CODE}"
        FORMATS ${_formats}
        NEEDS_CURL FALSE
        NEEDS_WEB_BROWSER FALSE)

    # Generate a JuceHeader.h for this target so <JuceHeader.h> can be included
    # from the plugin sources.
    juce_generate_juce_header(${target})

    # juce_audio_utils brings in the whole AudioProcessor scaffolding the
    # plugin is built on (core -> events -> audio_processors chain resolved
    # automatically), and the recommended flags keep builds consistent.
    target_link_libraries(${target}
        PRIVATE juce::juce_audio_utils
        PUBLIC  juce::juce_recommended_config_flags
                juce::juce_recommended_warning_flags)

    # Disable the bits of JUCE we don't need so nothing pulls in extra system
    # dependencies. These need to be PUBLIC so they propagate to the VST3 and
    # Standalone wrapper targets as well.
    target_compile_definitions(${target}
        PUBLIC
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0)

    if(ARG_SOURCES)
        target_sources(${target} PRIVATE ${ARG_SOURCES})
    endif()
endfunction()