FILE(GLOB_RECURSE app_sources 
    #main app
    ./src/*.c*
    )

set(BOARD "raspberry_pi_pico" CACHE STRING "" FORCE)
set(FAMILY "rp2040")
set(FREERTOS_PORT "GCC_RP2040" CACHE STRING "" FORCE)
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM
INTERFACE
${CMAKE_CURRENT_LIST_DIR}
)

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

target_compile_definitions (PicoApp PRIVATE
	PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1
)

# target_compile_definitions(PicoApp PUBLIC CFG_TUSB_CONFIG_FILE="Usb/tusb_config.h")

add_compile_options(
    ### Gnu/Clang C Options
    $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-fcolor-diagnostics>

    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wextra>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Werror>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Weverything>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-unused-variable>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-unused-parameter>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-type-limits>
)

if (WIN32)
add_custom_command(TARGET PicoApp POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E echo "Build succeeded! "
 COMMAND /bin/bash ${CMAKE_HOME_DIRECTORY}/tools/win64/flash.bat $<TARGET_FILE:PicoApp>
)
else()

if(NOT BUILD_TESTS)
add_custom_command(TARGET PicoApp POST_BUILD
COMMAND ${CMAKE_COMMAND} -E echo "Build succeeded!"
COMMAND /bin/bash ${CMAKE_HOME_DIRECTORY}/tools/linux/flash.sh $<TARGET_FILE:PicoApp>
)
endif()
endif()

# MACRO(HEADER_DIRECTORIES return_list)
#     FILE(GLOB_RECURSE new_list *.h*)
#     SET(dir_list "")
#     FOREACH(file_path ${new_list})
#         GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
#         SET(dir_list ${dir_list} ${dir_path})
#     ENDFOREACH()
#     LIST(REMOVE_DUPLICATES dir_list)
#     SET(${return_list} ${dir_list})
# ENDMACRO()

# pico_generate_pio_header(PicoApp ${CMAKE_HOME_DIRECTORY}/pio/blink.pio)
pico_generate_pio_header(PicoApp ${CMAKE_HOME_DIRECTORY}/pio/stepper.pio)

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
# pico_enable_stdio_uart(StepperPowerFeed 0)