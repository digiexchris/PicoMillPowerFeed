cmake_minimum_required(VERSION 3.21)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)


set(FREERTOS_KERNEL_PATH ${CMAKE_HOME_DIRECTORY}/src/FreeRTOS-Kernel)

# Enable exception handling
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(PICO_CXX_ENABLE_RTTI 1)


# Check for PICO_SDK_PATH in CMake cache first, then environment variable
if(DEFINED PICO_SDK_PATH)
  include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
elseif(DEFINED ENV{PICO_SDK_PATH})
  include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)
else()
  message(FATAL_ERROR "PICO_SDK_PATH not found. Set it as a CMake variable or environment variable to the path of your Pico SDK installation.")
endif()

project(PicoProject C CXX ASM)


# Check for ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")

  # Check if ccache supports the -Werror flag
  check_c_compiler_flag("-Werror" CCACHE_WERROR_SUPPORTED)
  check_cxx_compiler_flag("-Werror" CCACHE_WERROR_SUPPORTED)

  if(CCACHE_WERROR_SUPPORTED)
    message(STATUS "ccache supports -Werror")
  else()
    message(WARNING "ccache does not fully support -Werror, disabling it for ccache.")
  endif()
endif()


pico_sdk_init()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -mthumb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-m0plus -mthumb")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mcpu=cortex-m0plus -mthumb")

set(FREERTOS_PORT "GCC_RP2040" CACHE STRING "" FORCE)
set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM
INTERFACE
./
)
target_compile_definitions(freertos_config
INTERFACE
    projCOVERAGE_TEST=0
)

add_library(littlefs INTERFACE)
target_sources(littlefs INTERFACE
${CMAKE_HOME_DIRECTORY}/src/lib/littlefs/lfs.c
${CMAKE_HOME_DIRECTORY}/src/lib/littlefs/lfs_util.c
)
target_include_directories(littlefs INTERFACE
${CMAKE_HOME_DIRECTORY}/src/lib/littlefs
)
target_compile_options(littlefs INTERFACE -Wno-unused-function -Wno-null-dereference)
target_link_libraries(littlefs INTERFACE hardware_flash)

set(app_sources 
    #main app
    ${CMAKE_HOME_DIRECTORY}/src/Common.cxx
    ${CMAKE_HOME_DIRECTORY}/src/Display.cxx
    ${CMAKE_HOME_DIRECTORY}/src/FreeRTOS_Helpers.c
    ${CMAKE_HOME_DIRECTORY}/src/main.cxx
    ${CMAKE_HOME_DIRECTORY}/src/Settings.cxx
    ${CMAKE_HOME_DIRECTORY}/src/drivers/display/ConsoleDisplay.cxx
    ${CMAKE_HOME_DIRECTORY}/src/drivers/display/SSD1306Display.cxx
    ${CMAKE_HOME_DIRECTORY}/src/drivers/stepper/PicoStepper.cxx
    ${CMAKE_HOME_DIRECTORY}/src/drivers/Switches.cxx
    ${CMAKE_HOME_DIRECTORY}/src/FreeRTOS_Helpers.c
    # USB Mass Storage driver files
    ${CMAKE_HOME_DIRECTORY}/src/drivers/USBMassStorage/LittleFSSettings.cxx
    ${CMAKE_HOME_DIRECTORY}/src/drivers/USBMassStorage/TinyUSBMSC.cxx
    ${CMAKE_HOME_DIRECTORY}/src/drivers/USBMassStorage/usb_descriptors.c
    )

include(FetchContent)
# # Ensure timestamp preservation during extraction
# set(FETCHCONTENT_FULLY_DISCONNECTED OFF)
# set(FETCHCONTENT_UPDATES_DISCONNECTED OFF)
# set(FETCHCONTENT_QUIET OFF)
set(DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# Create a config update script
file(WRITE "${CMAKE_BINARY_DIR}/update_config.cmake" "
file(READ \"${CMAKE_SOURCE_DIR}/src/config/config.json\" CONFIG_CONTENT)
configure_file(
    \"${CMAKE_SOURCE_DIR}/src/config/config.h.in\"
    \"${CMAKE_BINARY_DIR}/include/config.h\"
    @ONLY
)
")

# Create a custom command to regenerate config.h whenever config.json changes
add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/include/config.h"
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/update_config.cmake"
    DEPENDS "${CMAKE_SOURCE_DIR}/src/config/config.json" "${CMAKE_SOURCE_DIR}/src/config/config.h.in"
    COMMENT "Updating config.h from config.json"
    VERBATIM
)

# Create a custom target that depends on the generated header
add_custom_target(generate_config_header DEPENDS "${CMAKE_BINARY_DIR}/include/config.h")

# Add include directory
include_directories(PUBLIC "${CMAKE_BINARY_DIR}/include")

# add_subdirectory(${FREERTOS_KERNEL_PATH})
# target_link_libraries(FreeRTOS-Kernel INTERFACE freertos_config)
add_subdirectory(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040 FreeRTOS-Kernel)

add_executable(PicoApp
${app_sources}
        )

# Make PicoApp depend on the config header target
add_dependencies(PicoApp generate_config_header)

target_compile_definitions (PicoApp PRIVATE
	PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1
  PICO_PLATFORM=${PICO_PLATFORM_VALUE}
  mainRUN_FREE_RTOS_ON_CORE=0
  USE_FREERTOS=1
  # PICO_STACK_SIZE=0x1000
)

# target_compile_definitions(PicoApp PUBLIC CFG_TUSB_CONFIG_FILE="tusb_config.h")

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
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-deprecated-declarations>
    ### C++ Specific
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-volatile>
    $<$<COMPILE_LANG_AND_ID:C,GNU>:-Wno-deprecated-declarations>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-deprecated-declarations>
)

add_custom_command(TARGET PicoApp POST_BUILD
COMMAND ${CMAKE_COMMAND} -E echo "Build succeeded!"
# COMMAND /bin/bash ${CMAKE_HOME_DIRECTORY}/tools/linux/flash.sh $<TARGET_FILE:PicoApp>
)

pico_generate_pio_header(PicoApp ${CMAKE_HOME_DIRECTORY}/src/pio/quadrature_encoder.pio)

add_subdirectory(${CMAKE_HOME_DIRECTORY}/src/lib/pico-ssd1306 pico-ssd1306)

include(FetchContent)
FetchContent_Declare(
    PIOStepperSpeedController
    GIT_REPOSITORY https://github.com/digiexchris/PIOStepperSpeedController.git
    GIT_TAG main
)
FetchContent_MakeAvailable(PIOStepperSpeedController)

target_link_libraries(PicoApp
pico_ssd1306
pico_multicore
hardware_i2c
hardware_flash
hardware_pio
hardware_sync
pico_bootrom
PIOStepperSpeedController
littlefs
nlohmann_json::nlohmann_json
tinyusb_device
tinyusb_board
FreeRTOS-Kernel
FreeRTOS-Kernel-Heap4
pico_stdlib)

target_include_directories(PicoApp PRIVATE
        ${CMAKE_HOME_DIRECTORY}
        ${header_dir_list}
        ${CMAKE_CURRENT_SOURCE_DIR})

pico_add_extra_outputs(PicoApp)

# Custom command to copy the UF2 file to the root of the project
add_custom_command(
  TARGET PicoApp POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/src/PicoApp.uf2
  ${CMAKE_SOURCE_DIR}/PicoApp.uf2
  COMMENT "Copying UF2 file to project root"
)

add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/compile_commands.json
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/compile_commands.json
  ${CMAKE_SOURCE_DIR}/compile_commands.json
  DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
  COMMENT "Copying compile_commands.json to src root"
)

if(TARGET PicoApp)  # Replace with your actual target name if different
    add_custom_target(flash_timestamp
      COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_BINARY_DIR}/flash_timestamp"
      DEPENDS PicoApp
      COMMENT "Updating flash timestamp"
    )
  endif()

# Create a custom target that always runs the above command
add_custom_target(copy_compile_commands_src ALL
  DEPENDS ${CMAKE_SOURCE_DIR}/compile_commands.json
)

add_dependencies(PicoApp copy_compile_commands_src)

# Use USB uart
pico_enable_stdio_usb(PicoApp 0)
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