FILE(GLOB_RECURSE app_sources 
    #main app
    ./src/*.c*
    )

option(PICO_PLATFORM_VALUE "PICO_PLATFORM_VALUE" "rp2040")
set(PICO_PLATFORM ${PICO_PLATFORM_VALUE} CACHE STRING "PICO_PLATFORM" FORCE)
message(STATUS "PICO_PLATFORM: ${PICO_PLATFORM}")
set(BOARD "raspberry_pi_pico" CACHE STRING "" FORCE)
set(FAMILY "rp2040")

# Disable auto-installing picotool
set(PICO_SDK_FETCH_FROM_GIT_AUTO OFF)
set(FREERTOS_PORT "GCC_RP2040" CACHE STRING "" FORCE)
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM
INTERFACE
${CMAKE_CURRENT_LIST_DIR}
)

add_library(littlefs INTERFACE)
target_sources(littlefs INTERFACE
${CMAKE_HOME_DIRECTORY}/lib/littlefs/lfs.c
${CMAKE_HOME_DIRECTORY}/lib/littlefs/lfs_util.c
)
target_include_directories(littlefs INTERFACE
${CMAKE_HOME_DIRECTORY}/lib/littlefs
)
target_compile_options(littlefs INTERFACE -Wno-unused-function -Wno-null-dereference)
target_link_libraries(littlefs INTERFACE hardware_flash)

target_compile_definitions(freertos_config
INTERFACE
    projCOVERAGE_TEST=0
)
set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)

# message( STATUS "FREERTOS_KERNEL_PATH: ${FREERTOS_KERNEL_PATH}" )

FILE(GLOB_RECURSE app_sources 
    #main app
    ./*.c*
    )

# Pull in FreeRTOS
include(${CMAKE_HOME_DIRECTORY}/FreeRTOS_Kernel_import.cmake)

add_executable(PicoApp
${app_sources}
        )

# Read the JSON file content
file(READ "${CMAKE_SOURCE_DIR}/config.json" CONFIG_CONTENT)

# Configure the header template
configure_file(
    "${CMAKE_SOURCE_DIR}/src/config.h.in"
    "${CMAKE_BINARY_DIR}/include/config.h"
    @ONLY
)

# Add include directory
target_include_directories(PicoApp PRIVATE "${CMAKE_BINARY_DIR}/include")

target_compile_definitions (PicoApp PRIVATE
	PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1
  PICO_PLATFORM=${PICO_PLATFORM_VALUE}
  mainRUN_FREE_RTOS_ON_CORE=1
  # PICO_STACK_SIZE=0x1000
)

# target_compile_definitions(PicoApp PUBLIC CFG_TUSB_CONFIG_FILE="Usb/tusb_config.h")

add_compile_options(
    ### Common Options
    $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fdiagnostics-color=always>
    
    ### Warning Options
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wextra>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Werror>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Weverything>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-unused-variable>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-unused-parameter>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-type-limits>
    
    ### C++ Specific
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-volatile>
)

include(FetchContent)

FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.gz
)

set(BOOST_INCLUDE_LIBRARIES json)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_MakeAvailable(Boost)

if (WIN32)
add_custom_command(TARGET PicoApp POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E echo "Build succeeded! "
 COMMAND /bin/bash ${CMAKE_HOME_DIRECTORY}/tools/win64/flash.bat $<TARGET_FILE:PicoApp>
)
else()

if(NOT BUILD_TESTS)
add_custom_command(TARGET PicoApp POST_BUILD
COMMAND ${CMAKE_COMMAND} -E echo "Build succeeded!"
# COMMAND /bin/bash ${CMAKE_HOME_DIRECTORY}/tools/linux/flash.sh $<TARGET_FILE:PicoApp>
)
endif()
endif()

pico_generate_pio_header(PicoApp ${CMAKE_HOME_DIRECTORY}/pio/stepper.pio)
pico_generate_pio_header(PicoApp ${CMAKE_HOME_DIRECTORY}/pio/quadrature_encoder.pio)

add_subdirectory(${CMAKE_HOME_DIRECTORY}/lib/pico-ssd1306 pico-ssd1306)
# add_subdirectory(${CMAKE_HOME_DIRECTORY}/lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src no-OS-FatFS-SDIO-SPI-RPi-Pico)
target_link_libraries(PicoApp
pico_ssd1306
pico_multicore
# you will also need hardware i2c library for communication with the display
hardware_i2c
# no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
hardware_flash
  hardware_sync
  littlefs
  tinyusb_additions
  tinyusb_board
  tinyusb_device
  Boost::json
pico_stdlib)

target_include_directories(PicoApp PRIVATE
        ${CMAKE_HOME_DIRECTORY}
        ${header_dir_list}
        ${CMAKE_CURRENT_SOURCE_DIR})

# add_subdirectory(${CMAKE_HOME_DIRECTORY}/lib/Simple3dStepperMovementPlanner StepTimingPlanner)

target_link_libraries(PicoApp pico_system_include_dirs pico_stdio_usb  FreeRTOS-Kernel FreeRTOS-Kernel-Heap4)
# target_link_libraries(PicoApp pico_system_include_dirs pico_stdio_usb )
pico_add_extra_outputs(PicoApp)

add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/src/compile_commands.json
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/compile_commands.json
  ${CMAKE_SOURCE_DIR}/src/compile_commands.json
  DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
  COMMENT "Copying compile_commands.json to src root"
)

# Create a custom target that always runs the above command
add_custom_target(copy_compile_commands_src ALL
  DEPENDS ${CMAKE_SOURCE_DIR}/src/compile_commands.json
)

add_dependencies(PicoApp copy_compile_commands_src)

# Use USB uart
pico_enable_stdio_usb(PicoApp 1)
pico_enable_stdio_uart(PicoApp 1)

find_program(OPENOCD openocd)
if(OPENOCD)
  add_custom_target(flash
    COMMAND ${OPENOCD} -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program ${CMAKE_PROJECT_NAME}.elf verify reset exit"
    DEPENDS ${CMAKE_PROJECT_NAME}
  )
  add_custom_target(reset COMMAND ${OPENOCD} -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c init -c reset -c exit)

  # add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()