cmake_minimum_required(VERSION 3.23)

project(openfiry VERSION 1.0.0 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "openfiry is a Directory Opus VFS plugin and can only be built on Windows.")
endif()

set(FIRY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/library/firy/source")

set(FIRY_SOURCES
    "${FIRY_SOURCE_DIR}/access/blocks.cpp"
    "${FIRY_SOURCE_DIR}/access/blocks.hpp"
    "${FIRY_SOURCE_DIR}/access/tracks.cpp"
    "${FIRY_SOURCE_DIR}/access/tracks.hpp"
    "${FIRY_SOURCE_DIR}/access/access.hpp"
    "${FIRY_SOURCE_DIR}/access/accessunit.hpp"
    "${FIRY_SOURCE_DIR}/buffer.cpp"
    "${FIRY_SOURCE_DIR}/buffer.hpp"
    "${FIRY_SOURCE_DIR}/console.cpp"
    "${FIRY_SOURCE_DIR}/console.hpp"
    "${FIRY_SOURCE_DIR}/filesystem/directory.hpp"
    "${FIRY_SOURCE_DIR}/filesystem/file.cpp"
    "${FIRY_SOURCE_DIR}/filesystem/file.hpp"
    "${FIRY_SOURCE_DIR}/filesystem/filesystem.cpp"
    "${FIRY_SOURCE_DIR}/filesystem/filesystem.hpp"
    "${FIRY_SOURCE_DIR}/filesystem/node.hpp"
    "${FIRY_SOURCE_DIR}/firy.cpp"
    "${FIRY_SOURCE_DIR}/firy.hpp"
    "${FIRY_SOURCE_DIR}/helpers/datetime.hpp"
    "${FIRY_SOURCE_DIR}/helpers/dirty.hpp"
    "${FIRY_SOURCE_DIR}/images/adf.cpp"
    "${FIRY_SOURCE_DIR}/images/adf.hpp"
    "${FIRY_SOURCE_DIR}/images/d64.cpp"
    "${FIRY_SOURCE_DIR}/images/d64.hpp"
    "${FIRY_SOURCE_DIR}/images/fat.cpp"
    "${FIRY_SOURCE_DIR}/images/fat.hpp"
    "${FIRY_SOURCE_DIR}/images/image.cpp"
    "${FIRY_SOURCE_DIR}/images/image.hpp"
    "${FIRY_SOURCE_DIR}/options.cpp"
    "${FIRY_SOURCE_DIR}/options.hpp"
    "${FIRY_SOURCE_DIR}/resources.cpp"
    "${FIRY_SOURCE_DIR}/resources.hpp"
    "${FIRY_SOURCE_DIR}/sources/source.cpp"
    "${FIRY_SOURCE_DIR}/sources/source.hpp"
    "${FIRY_SOURCE_DIR}/sources/sourcefile.cpp"
    "${FIRY_SOURCE_DIR}/sources/sourcefile.hpp"
)

add_library(firy STATIC ${FIRY_SOURCES})

target_include_directories(firy PUBLIC "${FIRY_SOURCE_DIR}")
target_compile_features(firy PUBLIC cxx_std_17)

set(OPENFIRY_SOURCES
    dllmain.cpp
    firyopus.cpp
    firyopus.hpp
    opusoptions.cpp
    opusoptions.hpp
    safecontainer.hpp
    stdafx.cpp
    stdafx.h
    "headers/plugin support.h"
    "headers/vfs plugins.h"
    "headers/viewer plugins.h"
)

add_library(openfiry SHARED ${OPENFIRY_SOURCES})

target_link_libraries(openfiry PRIVATE firy)
target_compile_features(openfiry PRIVATE cxx_std_17)
target_include_directories(openfiry PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/headers"
)
target_compile_definitions(openfiry PRIVATE
    OPENFIRY_EXPORTS
    _WINDOWS
    _USRDLL
    LITT_ENDIAN
    DOPUS_PLUGIN_HELPER
    _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
    UNICODE
    _UNICODE
)

set_target_properties(openfiry PROPERTIES
    OUTPUT_NAME "OPENFIRY"
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/bin/$<CONFIG>>"
    LIBRARY_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/bin/$<CONFIG>>"
    ARCHIVE_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/lib/$<CONFIG>>"
    PDB_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/bin/$<CONFIG>>"
)

set_target_properties(firy PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/lib/$<CONFIG>>"
    COMPILE_PDB_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/pdb/$<CONFIG>>"
)

if(MSVC)
    target_compile_options(firy PRIVATE /W3 /sdl /permissive-)
    target_compile_options(openfiry PRIVATE /W3 /sdl /permissive- $<$<CONFIG:Release>:/Zi>)
    target_link_options(openfiry PRIVATE /SUBSYSTEM:WINDOWS $<$<CONFIG:Release>:/DEBUG>)
endif()

source_group(TREE "${FIRY_SOURCE_DIR}" PREFIX "library/firy/source" FILES ${FIRY_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${OPENFIRY_SOURCES})

install(TARGETS openfiry
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
)
