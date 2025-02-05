FILE(GLOB_RECURSE app_sources 
    #main app
    ./src/*.c*
    )

# option(PICO_PLATFORM_VALUE "PICO_PLATFORM_VALUE" "rp2040")
# set(PICO_PLATFORM ${PICO_PLATFORM_VALUE} CACHE STRING "PICO_PLATFORM" FORCE)
# message(STATUS "PICO_PLATFORM: ${PICO_PLATFORM}")
# set(BOARD "raspberry_pi_pico" CACHE STRING "" FORCE)
# set(FAMILY "rp2040")

# # Disable auto-installing picotool
# set(PICO_SDK_FETCH_FROM_GIT_AUTO OFF)
# set(FREERTOS_PORT "GCC_RP2040" CACHE STRING "" FORCE)
# add_library(freertos_config INTERFACE)
# target_include_directories(freertos_config SYSTEM
# INTERFACE
# ${CMAKE_CURRENT_LIST_DIR}
# )


# message( STATUS "FREERTOS_KERNEL_PATH: ${FREERTOS_KERNEL_PATH}" )

FILE(GLOB_RECURSE app_sources 
    #main app
    ./*.c*
    )

# Pull in FreeRTOS
# include(${CMAKE_HOME_DIRECTORY}/FreeRTOS_Kernel_import.cmake)

add_executable(${EXE_NAME}
${app_sources}
        )


  add_custom_command(
          TARGET ${EXE_NAME}
          POST_BUILD
          COMMAND ${CMAKE_OBJCOPY} ARGS -S -O binary ${CMAKE_BINARY_DIR}/${EXE_NAME}.elf ${CMAKE_BINARY_DIR}/${EXE_NAME}.bin
  )
  
#   target_link_libraries(${EXE_NAME} 
#   sched
#   drivers
#   boards
#   c
#   mm
#   arch
#   xx
#   apps
#   fs
#   binfmt
#   board
#   gcc
#   supc++
#   uClibc++
#  nxfonts
#   )

# add_compile_options(
#     ### Common Options
#     $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
#     $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fdiagnostics-color=always>
    
#     ### Warning Options
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall>
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wextra>
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Werror>
#     $<$<COMPILE_LANG_AND_ID:C,Clang>:-Weverything>
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-unused-variable>
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-unused-parameter>
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-type-limits>
    
#     ### C++ Specific
#     $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-volatile>
# )


# if(NOT BUILD_TESTS)
# add_custom_command(TARGET ${EXE_NAME} POST_BUILD
# COMMAND ${CMAKE_COMMAND} -E echo "Build succeeded!"
# # COMMAND /bin/bash ${CMAKE_HOME_DIRECTORY}/tools/linux/flash.sh $<TARGET_FILE:PicoApp>
# )
# endif()

# pico_generate_pio_header(PicoApp ${CMAKE_HOME_DIRECTORY}/pio/stepper.pio)
# pico_generate_pio_header(PicoApp ${CMAKE_HOME_DIRECTORY}/pio/quadrature_encoder.pio)

# add_subdirectory(${CMAKE_HOME_DIRECTORY}/lib/pico-ssd1306 pico-ssd1306)
# add_subdirectory(${CMAKE_HOME_DIRECTORY}/lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src no-OS-FatFS-SDIO-SPI-RPi-Pico)
# target_link_libraries(PicoApp
# # pico_ssd1306
# pico_multicore
# # you will also need hardware i2c library for communication with the display
# hardware_i2c
# # no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
# pico_stdlib)

message(STATUS "Pico sdk path is ${PICO_SDK_PATH}")
include(${NUTTX_PATH}/scripts/target.cmake)
include(${NUTTX_PATH}/scripts/toolchain.cmake)
target_include_directories(${EXE_NAME} PRIVATE
        ${CMAKE_HOME_DIRECTORY}
        ${header_dir_list}
        ${CMAKE_CURRENT_SOURCE_DIR})



# add_subdirectory(${CMAKE_HOME_DIRECTORY}/lib/Simple3dStepperMovementPlanner StepTimingPlanner)

# target_link_libraries(PicoApp pico_system_include_dirs pico_stdio_usb  FreeRTOS-Kernel FreeRTOS-Kernel-Heap4)
# # target_link_libraries(PicoApp pico_system_include_dirs pico_stdio_usb )
# pico_add_extra_outputs(PicoApp)

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

add_dependencies(${EXE_NAME} copy_compile_commands_src)

# Use USB uart
# pico_enable_stdio_usb(PicoApp 1)
# pico_enable_stdio_uart(StepperPowerFeed 0)