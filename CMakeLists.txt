cmake_minimum_required(VERSION 3.13)

message(STATUS "BUILD_PICO_APP is ${BUILD_PICO_APP}")

option(BUILD_PICO_APP "Build the Pico application" ON)
option(BUILD_TESTS "Build the tests" OFF)


include(FetchContent)

set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "Build rapidjson tests" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "Build rapidjson examples" FORCE)
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "Build rapidjson documentation" FORCE)

FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG v1.1.0
)
FetchContent_GetProperties(rapidjson)
if(NOT rapidjson_POPULATED)
  FetchContent_Populate(rapidjson)
endif()

# Create an interface target for rapidjson
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${rapidjson_SOURCE_DIR}/include)

# Disable -Wambiguous-reversed-operator for rapidjson
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(rapidjson INTERFACE -Wno-ambiguous-reversed-operator)
endif()


# Read the JSON file content
file(READ "${CMAKE_SOURCE_DIR}/config.json" CONFIG_CONTENT)

# Configure the header template
configure_file(
    "${CMAKE_SOURCE_DIR}/src/config.h.in"
    "${CMAKE_BINARY_DIR}/include/config.h"
    @ONLY
)

# Add include directory
include_directories(PUBLIC "${CMAKE_BINARY_DIR}/include")

if(BUILD_PICO_APP)

  include(${CMAKE_HOME_DIRECTORY}/pico_sdk_import.cmake)

  

  project(PicoProject C CXX ASM)

  set(PICO_BOARD pico CACHE STRING "Board type")
  
  pico_sdk_init()

  add_compile_options(
      ### Gnu/Clang C Options
      $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
      $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fdiagnostics-color=always>

      # $<$<COMPILE_LANG_AND_ID:C,Clang>:-fcolor-diagnostics>
  )

  # include(FetchContent)

  # FetchContent_Declare(
  #   Simple3dStepperMovementPlanner
  #   GIT_REPOSITORY https://github.com/digiexchris/Simple3dStepperMovementPlanner.git
  #   GIT_TAG main
  # )

  # FetchContent_MakeAvailable(Simple3dStepperMovementPlanner)



  # Create an interface target to set system include directories for Pico SDK libraries
  add_library(pico_system_include_dirs INTERFACE)
  target_include_directories(pico_system_include_dirs SYSTEM INTERFACE ${PICO_INCLUDE_DIRS})

  # Include directories for other Pico SDK libraries
  target_link_libraries(pico_system_include_dirs INTERFACE pico_stdlib hardware_pio hardware_gpio pico_multicore pico_stdio_usb pico_printf pico_async_context_threadsafe_background
  )

  # Add subdirectories
  add_subdirectory(src)
else()
  project(PicoProject C CXX ASM)
endif()

# Optionally add tests
option(BUILD_TESTS "Build tests" OFF)
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()



