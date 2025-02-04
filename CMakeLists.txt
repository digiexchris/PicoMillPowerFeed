cmake_minimum_required(VERSION 3.13)

message(STATUS "BUILD_PICO_APP is ${BUILD_PICO_APP}")

option(BUILD_PICO_APP "Build the Pico application" ON)
option(BUILD_TESTS "Build the tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)

if(BUILD_PICO_APP)

#   include(${CMAKE_HOME_DIRECTORY}/pico_sdk_import.cmake)

  

  project(PicoProject C CXX ASM)

  set(NUTTX_PATH "${CMAKE_SOURCE_DIR}/nuttx-export-12.8.0")

  include(cmake/raspberrypi-pico.cmake)
  
  set(AC_COMMON_FLAGS "${AC_COMMON_FLAGS} -Wall -Wshadow -Wundef -fno-strict-aliasing -Os")
  set(AC_COMMON_FLAGS "${AC_COMMON_FLAGS} -D_DEBUG -D_LIBCPP_BUILD_STATIC -D_LIBCPP_NO_EXCEPTIONS ")
  set(AC_COMMON_FLAGS "${AC_COMMON_FLAGS} -fcheck-new -fno-rtti -pedantic ")
  set(AC_COMMON_FLAGS "${AC_COMMON_FLAGS} -nostdinc++")
  
  set(AC_DEFINES "${AC_DEFINES} -DCONFIG_WCHAR_BUILTIN")
  
  include_directories(
          src
          ${NUTTX_PATH}/include
          ${NUTTX_PATH}/include/libcxx
          ${NUTTX_PATH}/arch/chip
  )
  
  set(EXE_NAME PicoMill)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${AC_HW_FLAGS} ${AC_DEFINES}")
  set(CMAKE_CXX_FLAGS     "${AC_HW_FLAGS} ${AC_DEFINES} ${AC_COMMON_FLAGS} ${AC_CXX_EXTRA_FLAGS}")
  if (PARAM_DEBUG)
      set(CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -g")
  endif()
  
  set(CMAKE_SKIP_RPATH ON)
  set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_LINKER} ${AC_LINKER_FLAGS} -o ${EXE_NAME}.elf <OBJECTS> <LINK_LIBRARIES>")
  set(BUILD_SHARED_LIBS OFF)
  add_subdirectory(src)
else()
  project(PicoProject C CXX ASM)
endif()

# Optionally add tests
option(BUILD_TESTS "Build tests" OFF)
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()



