cmake_minimum_required(VERSION 3.22)

option(BUILD_PICO_APP "Build the Pico application" ON)
option(BUILD_TESTS "Build the tests" OFF)

# Add include directory
include_directories(PUBLIC "${CMAKE_BINARY_DIR}/include")

if(BUILD_PICO_APP)
  # Add subdirectories
  add_subdirectory(src)
else()
  
endif()

if (BUILD_TESTS)
    enable_testing()
    project(PicoProject C CXX ASM)
    add_subdirectory(tests)
endif()



