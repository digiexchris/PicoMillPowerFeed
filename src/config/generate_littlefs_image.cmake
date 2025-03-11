# CMake script to generate a LittleFS filesystem image with default config.json
# This runs at build time to create a filesystem image that will be embedded in the final binary

set(LITTLEFS_IMAGE_SIZE 256K)  # Match the FS_SIZE in LittleFSSettings.hxx

# Create a temporary directory for building the filesystem
set(FS_BUILD_DIR "${CMAKE_BINARY_DIR}/fs_image")
file(MAKE_DIRECTORY ${FS_BUILD_DIR})

# Generate the default config.json and write it to the temp directory
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/config.json"
    "${FS_BUILD_DIR}/config.json"
    COPYONLY
)

# Find mklittlefs or build it if not found
find_program(MKLITTLEFS_EXECUTABLE mklittlefs)
if(NOT MKLITTLEFS_EXECUTABLE)
    message(STATUS "mklittlefs not found, using Python littlefs-tools instead")
    
    # Check for Python and littlefs-tools package
    find_package(Python3 REQUIRED)
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import littlefs"
        RESULT_VARIABLE LITTLEFS_PYTHON_RESULT
        ERROR_QUIET
    )
    
    if(NOT LITTLEFS_PYTHON_RESULT EQUAL 0)
        message(FATAL_ERROR "Python littlefs package not found. Install with: pip install littlefs-tools")
    endif()
    
    # Create a Python script to create the image
    set(LITTLEFS_SCRIPT "${CMAKE_BINARY_DIR}/create_littlefs_image.py")
    file(WRITE ${LITTLEFS_SCRIPT} "
import littlefs
import sys
import os

# Create a filesystem image with the specified size
fs = littlefs.LittleFS(block_size=4096, block_count=${LITTLEFS_IMAGE_SIZE}/4096)

# Add config.json to the image
with open('${FS_BUILD_DIR}/config.json', 'rb') as f:
    content = f.read()
    fs.mkdir('/')
    fs.write('/config.json', content)

# Write the filesystem image to a file
with open('${CMAKE_BINARY_DIR}/fs_image.bin', 'wb') as f:
    f.write(fs.export())
")

    # Execute the Python script
    execute_process(
        COMMAND ${Python3_EXECUTABLE} ${LITTLEFS_SCRIPT}
        RESULT_VARIABLE LITTLEFS_CREATE_RESULT
        ERROR_VARIABLE LITTLEFS_CREATE_ERROR
    )
    
    if(NOT LITTLEFS_CREATE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to create LittleFS image: ${LITTLEFS_CREATE_ERROR}")
    endif()
else()
    # Use mklittlefs directly
    execute_process(
        COMMAND ${MKLITTLEFS_EXECUTABLE} -c ${FS_BUILD_DIR} -s ${LITTLEFS_IMAGE_SIZE} -b 4096 ${CMAKE_BINARY_DIR}/fs_image.bin
        RESULT_VARIABLE LITTLEFS_CREATE_RESULT
        ERROR_VARIABLE LITTLEFS_CREATE_ERROR
    )
    
    if(NOT LITTLEFS_CREATE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to create LittleFS image: ${LITTLEFS_CREATE_ERROR}")
    endif()
endif()

# Convert the binary image to a C header file
file(READ ${CMAKE_BINARY_DIR}/fs_image.bin HEX_CONTENT HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1, " FORMATTED_HEX "${HEX_CONTENT}")

# Write the header file
file(WRITE ${CMAKE_BINARY_DIR}/fs_image.h "
#pragma once

// Auto-generated LittleFS filesystem image with default config.json
// Generated during build - DO NOT EDIT

#include <stdint.h>
#include <stddef.h>

// Image is ${LITTLEFS_IMAGE_SIZE} bytes
#define FS_IMAGE_SIZE ${LITTLEFS_IMAGE_SIZE}

const uint8_t fs_image[] = {
${FORMATTED_HEX}
};
")

message(STATUS "LittleFS image generated with default config at ${CMAKE_BINARY_DIR}/fs_image.bin")
message(STATUS "LittleFS header generated at ${CMAKE_BINARY_DIR}/fs_image.h")