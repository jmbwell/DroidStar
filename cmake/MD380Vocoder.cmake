set(USE_MD380_VOCODER "AUTO" CACHE STRING "External MD380 vocoder: AUTO, ON, or OFF")
set_property(CACHE USE_MD380_VOCODER PROPERTY TYPE STRING)
set_property(CACHE USE_MD380_VOCODER PROPERTY STRINGS AUTO ON OFF)
string(TOUPPER "${USE_MD380_VOCODER}" _md380_selection)
if(NOT _md380_selection MATCHES "^(AUTO|ON|OFF)$")
    message(FATAL_ERROR "USE_MD380_VOCODER must be AUTO, ON, or OFF")
endif()

# Function scope keeps probe settings out of the application's configuration.
function(check_md380_vocoder result)
    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_INCLUDES "${MD380_VOCODER_INCLUDE_DIR}")
    set(CMAKE_REQUIRED_LIBRARIES "${MD380_VOCODER_LIBRARY}")
    # A static-library-only probe would miss wrong architectures and missing symbols.
    set(CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE)
    # Recheck on configure: the selected library or target flags may have changed.
    unset(MD380_VOCODER_LINKS CACHE)
    unset(MD380_VOCODER_LINKS)
    check_cxx_source_compiles([[
        #include <md380_vocoder.h>
        int main() {
            uint8_t ambe[9] = {};
            int16_t pcm[160] = {};
            md380_init();
            md380_encode(ambe, pcm);
            md380_decode(ambe, pcm);
            md380_encode_fec(ambe, pcm);
            md380_decode_fec(ambe, pcm);
            return 0;
        }
    ]] MD380_VOCODER_LINKS)
    set(${result} "${MD380_VOCODER_LINKS}" PARENT_SCOPE)
endfunction()

set(MD380_VOCODER_AVAILABLE FALSE)
if(_md380_selection STREQUAL "OFF")
    message(STATUS "Vocoder: bundled (MD380 explicitly disabled)")
else()
    find_path(MD380_VOCODER_INCLUDE_DIR NAMES md380_vocoder.h)
    find_library(MD380_VOCODER_LIBRARY NAMES md380_vocoder)
    if(MD380_VOCODER_INCLUDE_DIR AND MD380_VOCODER_LIBRARY)
        check_md380_vocoder(MD380_VOCODER_AVAILABLE)
        set(_md380_reason "header/library failed the target compile and link check")
    else()
        set(_md380_reason "header or library not found")
    endif()

    if(MD380_VOCODER_AVAILABLE)
        message(STATUS "Vocoder: MD380 (${MD380_VOCODER_LIBRARY})")
    elseif(_md380_selection STREQUAL "ON")
        message(FATAL_ERROR
            "USE_MD380_VOCODER=ON, but MD380 cannot be used: ${_md380_reason}. "
            "Install a target-compatible library and set CMAKE_PREFIX_PATH, or set "
            "MD380_VOCODER_INCLUDE_DIR and MD380_VOCODER_LIBRARY explicitly. "
            "Use AUTO for automatic fallback or OFF for the bundled codecs.")
    else()
        message(STATUS "Vocoder: bundled (MD380 ${_md380_reason})")
    endif()
endif()
