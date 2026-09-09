# Builders can supply their own identity without changing the project's defaults.
set(DROIDSTAR_BUNDLE_IDENTIFIER "com.yourcompany.90d5dd37.DroidStar" CACHE STRING
    "Apple bundle identifier (override for your own signed builds)")
set(DROIDSTAR_BUILD_NUMBER "1" CACHE STRING "Numeric Apple bundle build number")
if(NOT DROIDSTAR_BUNDLE_IDENTIFIER MATCHES "^[A-Za-z0-9-]+(\\.[A-Za-z0-9-]+)+$")
    message(FATAL_ERROR "DROIDSTAR_BUNDLE_IDENTIFIER must be a reverse-domain identifier")
endif()
if(NOT DROIDSTAR_BUILD_NUMBER MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR "DROIDSTAR_BUILD_NUMBER must be a positive integer")
endif()
set(DROIDSTAR_MICROPHONE_DESCRIPTION
    "DroidStar uses your microphone to transmit your voice to amateur radio networks.")
set_target_properties(DroidStar PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "${DROIDSTAR_BUNDLE_IDENTIFIER}"
    MACOSX_BUNDLE_BUNDLE_NAME "DroidStar"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${DROIDSTAR_BUILD_NUMBER}"
)
if(IOS)
    enable_language(OBJCXX)
    target_sources(DroidStar PRIVATE iosaudiosession.mm iosaudiosession.h)
    # Qt's prebuilt iOS Multimedia package requires bundled FFmpeg frameworks.
    if(COMMAND qt_add_ios_ffmpeg_libraries)
        qt_add_ios_ffmpeg_libraries(DroidStar)
    endif()
    target_sources(DroidStar PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Images.xcassets")
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/Images.xcassets"
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_target_properties(DroidStar PROPERTIES
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
        XCODE_ATTRIBUTE_GENERATE_INFOPLIST_FILE YES
        XCODE_ATTRIBUTE_MARKETING_VERSION "${PROJECT_VERSION}"
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "${DROIDSTAR_BUILD_NUMBER}"
        XCODE_ATTRIBUTE_INFOPLIST_KEY_NSMicrophoneUsageDescription "${DROIDSTAR_MICROPHONE_DESCRIPTION}")
else()
    set_target_properties(DroidStar PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")
endif()
