cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

################################################################################
# Create library metadata
################################################################################

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Deployment target for OSX" FORCE)
    set(CMAKE_POSITION_INDEPENDENT_CODE FALSE)
endif()

project(reaper_drivenbymoss VERSION 25.0.0.0 LANGUAGES CXX)

string(REPLACE "." "," VER_VERSION "${PROJECT_VERSION}")
string(TIMESTAMP VER_YEAR "%Y" UTC)
add_definitions(-DVER_VERSION=${VER_VERSION})
add_definitions(-DVER_VERSION_STR="${PROJECT_VERSION}")
add_definitions(-DVER_YEAR="${VER_YEAR}")

################################################################################
# Set C++ version and add libraries
################################################################################

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# Must be after project definition otherwise CMAKE_FIND_LIBRARY_PREFIXES / CMAKE_FIND_LIBRARY_SUFFIXES are not set!
find_package(Java REQUIRED)
find_package(JNI REQUIRED)
include(UseJava)

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    message(STATUS "COCOA_LIBRARY is ${COCOA_LIBRARY}")
endif()

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################

if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

################################################################################
# Global configuration types
################################################################################

set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)

################################################################################
# Global compiler options
################################################################################

if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
endif()

################################################################################
# Global linker options
################################################################################

if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
endif()

################################################################################
# Common utils
################################################################################
include(Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "../reaper_drivenbymoss/ActionProcessor.h"
    "../reaper_drivenbymoss/afxres.h"
    "../reaper_drivenbymoss/ClipProcessor.h"
    "../reaper_drivenbymoss/CodeAnalysis.h"
    "../reaper_drivenbymoss/Collectors.h"
    "../reaper_drivenbymoss/DataCollector.h"
    "../reaper_drivenbymoss/de_mossgrabers_reaper_MainApp.h"
    "../reaper_drivenbymoss/DeviceProcessor.h"
    "../reaper_drivenbymoss/EqDeviceProcessor.h"
    "../reaper_drivenbymoss/dllmain.h"
    "../reaper_drivenbymoss/DrivenByMossSurface.h"
    "../reaper_drivenbymoss/FunctionExecutor.h"
    "../reaper_drivenbymoss/GrooveProcessor.h"
    "../reaper_drivenbymoss/IniFileProcessor.h"
    "../reaper_drivenbymoss/JvmManager.h"
    "../reaper_drivenbymoss/Marker.h"
    "../reaper_drivenbymoss/MarkerProcessor.h"
    "../reaper_drivenbymoss/MastertrackProcessor.h"
    "../reaper_drivenbymoss/Model.h"
    "../reaper_drivenbymoss/NoteRepeatProcessor.h"
    "../reaper_drivenbymoss/OscParser.h"
    "../reaper_drivenbymoss/OscProcessor.h"
    "../reaper_drivenbymoss/Parameter.h"
    "../reaper_drivenbymoss/ProjectProcessor.h"
    "../reaper_drivenbymoss/ReaDebug.h"
    "../reaper_drivenbymoss/reaper_plugin.h"
    "../reaper_drivenbymoss/reaper_plugin_functions.h"
    "../reaper_drivenbymoss/ReaperUtils.h"
    "../reaper_drivenbymoss/resource.h"
    "../reaper_drivenbymoss/SceneProcessor.h"
    "../reaper_drivenbymoss/Send.h"
    "../reaper_drivenbymoss/stdafx.h"
    "../reaper_drivenbymoss/StringUtils.h"
    "../reaper_drivenbymoss/targetver.h"
    "../reaper_drivenbymoss/Track.h"
    "../reaper_drivenbymoss/TrackProcessor.h"
    "../reaper_drivenbymoss/TransportProcessor.h"
    "../reaper_drivenbymoss/WrapperGSL.h"
    "../reaper_drivenbymoss/WrapperJNI.h"
    "../reaper_drivenbymoss/WrapperReaperFunctions.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "../reaper_drivenbymoss/ActionProcessor.cpp"
    "../reaper_drivenbymoss/ClipProcessor.cpp"
    "../reaper_drivenbymoss/DataCollector.cpp"
    "../reaper_drivenbymoss/DeviceProcessor.cpp"
    "../reaper_drivenbymoss/EqDeviceProcessor.cpp"
    "../reaper_drivenbymoss/dllmain.cpp"
    "../reaper_drivenbymoss/DrivenByMossSurface.cpp"
    "../reaper_drivenbymoss/FunctionExecutor.cpp"
    "../reaper_drivenbymoss/GrooveProcessor.cpp"
    "../reaper_drivenbymoss/IniFileProcessor.cpp"
    "../reaper_drivenbymoss/JvmManager.cpp"
    "../reaper_drivenbymoss/Marker.cpp"
    "../reaper_drivenbymoss/MarkerProcessor.cpp"
    "../reaper_drivenbymoss/MastertrackProcessor.cpp"
    "../reaper_drivenbymoss/Model.cpp"
    "../reaper_drivenbymoss/NoteRepeatProcessor.cpp"
    "../reaper_drivenbymoss/OscParser.cpp"
    "../reaper_drivenbymoss/Parameter.cpp"
    "../reaper_drivenbymoss/ProjectProcessor.cpp"
    "../reaper_drivenbymoss/ReaDebug.cpp"
    "../reaper_drivenbymoss/ReaperUtils.cpp"
    "../reaper_drivenbymoss/SceneProcessor.cpp"
    "../reaper_drivenbymoss/Send.cpp"
    "../reaper_drivenbymoss/stdafx.cpp"
    "../reaper_drivenbymoss/StringUtils.cpp"
    "../reaper_drivenbymoss/Track.cpp"
    "../reaper_drivenbymoss/TrackProcessor.cpp"
)
source_group("Source Files" FILES ${Source_Files})

if(WIN32)
    set(Source_Files ${Source_Files} "../reaper_drivenbymoss/res.rc")
endif()

if(APPLE)
    set(Source_Files ${Source_Files} "../libraries/WDL/swell/swell-modstub.mm")
endif()

if(UNIX AND NOT APPLE)
    set(Source_Files ${Source_Files} "../libraries/WDL/swell/swell-modstub-generic.cpp")
endif()

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################

if(WIN32)
    add_library(${PROJECT_NAME} MODULE ${ALL_FILES})
else()
    add_library(${PROJECT_NAME} SHARED ${ALL_FILES})
endif()

if(NOT WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
endif()

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE reaper_drivenbymoss)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_GLOBAL_KEYWORD "Win32Proj"
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

################################################################################
# MSVC runtime library
################################################################################

if(MSVC)
    get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:Debug>:
            MultiThreadedDebug
        >
        $<$<CONFIG:Release>:
            MultiThreaded
        >
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})
endif()

################################################################################
# Include directories
################################################################################

include_directories(${JNI_INCLUDE_DIRS} ${_classDir} ${_stubDir})

target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../libraries/GSL;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../libraries/WDL;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../reaper_drivenbymoss;"
)

if(NOT WIN32)
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../libraries/WDL/swell;"
    )
endif()

################################################################################
# Compile definitions
################################################################################

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG;"
            "OSC_HOST_LITTLE_ENDIAN"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_WINDOWS;"
        "_USRDLL;"
        "reaper_drivenbymoss_EXPORTS;"
        "UNICODE;"
        "_UNICODE"
    )
endif()

if(NOT WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "SWELL_PROVIDED_BY_APP"
    )
endif()

if(UNIX AND NOT APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "LINUX"
    )
endif()


################################################################################
# Compile and link options
################################################################################

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od
        >
        $<$<CONFIG:Release>:
            /O2;
            /Oi;
            /Gy
        >
        /std:c++14;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /INCREMENTAL
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO
        >
        /DEBUG:FULL;
        /SUBSYSTEM:WINDOWS;
        /NXCOMPAT:NO;
        /DYNAMICBASE:NO
    )
endif()

################################################################################
# Dependencies
################################################################################

if(MSVC)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "kernel32"
        "user32"
        "gdi32"
        "winspool"
        "comdlg32"
        "advapi32"
        "shell32"
        "ole32"
        "oleaut32"
        "uuid"
        "odbc32"
        "odbccp32"
    )
    target_link_libraries(${PROJECT_NAME} ${ADDITIONAL_LIBRARY_DEPENDENCIES})
endif()

if(APPLE)
    target_link_libraries(${PROJECT_NAME} ${COCOA_LIBRARY})
endif()
