cmake_minimum_required(VERSION 3.21)


# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(FREERTOS_KERNEL_PATH ${CMAKE_HOME_DIRECTORY}/FreeRTOS-Kernel)
# option(PICO_PLATFORM_VALUE "PICO_PLATFORM_VALUE" "rp2040")
# set(PICO_PLATFORM ${PICO_PLATFORM_VALUE} "rp2040")
# message(STATUS "PICO_PLATFORM: ${PICO_PLATFORM}")
# set(BOARD "raspberry_pi_pico" CACHE STRING "" FORCE)
# set(FAMILY "rp2040")

# Enable exception handling
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(PICO_CXX_ENABLE_RTTI 1)

# Disable auto-installing picotool
# set(PICO_SDK_FETCH_FROM_GIT_AUTO OFF)
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
${CMAKE_HOME_DIRECTORY}/lib/littlefs/lfs.c
${CMAKE_HOME_DIRECTORY}/lib/littlefs/lfs_util.c
)
target_include_directories(littlefs INTERFACE
${CMAKE_HOME_DIRECTORY}/lib/littlefs
)
target_compile_options(littlefs INTERFACE -Wno-unused-function -Wno-null-dereference)
target_link_libraries(littlefs INTERFACE hardware_flash)



# message( STATUS "FREERTOS_KERNEL_PATH: ${FREERTOS_KERNEL_PATH}" )

set(app_sources 
    #main app
    ${CMAKE_HOME_DIRECTORY}/Common.cxx
    ${CMAKE_HOME_DIRECTORY}/Display.cxx
    ${CMAKE_HOME_DIRECTORY}/FreeRTOS_Helpers.c
    ${CMAKE_HOME_DIRECTORY}/MachineState.cxx
    ${CMAKE_HOME_DIRECTORY}/main.cxx
    ${CMAKE_HOME_DIRECTORY}/Settings.cxx
    ${CMAKE_HOME_DIRECTORY}/StepperState.cxx
    ${CMAKE_HOME_DIRECTORY}/drivers/display/ConsoleDisplay.cxx
    ${CMAKE_HOME_DIRECTORY}/drivers/display/SSD1306Display.cxx
    ${CMAKE_HOME_DIRECTORY}/drivers/stepper/PicoStepper.cxx
    ${CMAKE_HOME_DIRECTORY}/drivers/Switches.cxx
    )




    include(FetchContent)

    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
    
    
    
    
    # Read the JSON file content
    file(READ "${CMAKE_SOURCE_DIR}/config/config.json" CONFIG_CONTENT)
    
    # Configure the header template
    configure_file(
        "${CMAKE_SOURCE_DIR}/config/config.h.in"
        "${CMAKE_BINARY_DIR}/include/config.h"
        @ONLY
    )
    
    # Add include directory
    include_directories(PUBLIC "${CMAKE_BINARY_DIR}/include")


    

include(${CMAKE_HOME_DIRECTORY}/pico_sdk_import.cmake)



# Pull in FreeRTOS



project(PicoProject C CXX ASM)
pico_sdk_init()
# add_subdirectory(${FREERTOS_KERNEL_PATH})
# target_link_libraries(FreeRTOS-Kernel INTERFACE freertos_config)
add_subdirectory(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040 FreeRTOS-Kernel)


add_executable(PicoApp
${app_sources}
        )


target_compile_definitions (PicoApp PRIVATE
	PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1
  PICO_PLATFORM=${PICO_PLATFORM_VALUE}
  mainRUN_FREE_RTOS_ON_CORE=0
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

pico_generate_pio_header(PicoApp ${CMAKE_HOME_DIRECTORY}/pio/quadrature_encoder.pio)

add_subdirectory(${CMAKE_HOME_DIRECTORY}/lib/pico-ssd1306 pico-ssd1306)

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
pico_async
hardware_i2c
hardware_flash
hardware_pio
hardware_sync
PIOStepperSpeedController
littlefs
nlohmann_json::nlohmann_json
# tinyusb_additions
# tinyusb_board
# tinyusb_device
FreeRTOS-Kernel
FreeRTOS-Kernel-Heap4 
pico_stdlib)

target_include_directories(PicoApp PRIVATE
        ${CMAKE_HOME_DIRECTORY}
        ${header_dir_list}
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(PicoApp pico_stdio_usb  FreeRTOS-Kernel FreeRTOS-Kernel-Heap4)
pico_add_extra_outputs(PicoApp)

include_directories(PUBLIC ${rapidjson_SOURCE_DIR}/include)

add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/compile_commands.json
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/compile_commands.json
  ${CMAKE_SOURCE_DIR}/compile_commands.json
  DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
  COMMENT "Copying compile_commands.json to src root"
)

# Create a custom target that always runs the above command
add_custom_target(copy_compile_commands_src ALL
  DEPENDS ${CMAKE_SOURCE_DIR}/compile_commands.json
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