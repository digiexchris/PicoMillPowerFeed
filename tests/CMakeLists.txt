set(CMAKE_MINIMUM_REQUIRED_VERSION 3.21)


set(CMAKE_BUILD_TYPE Debug) 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fno-omit-frame-pointer")

    # Read the JSON file content
    file(READ "${CMAKE_SOURCE_DIR}/src/config/config.json" CONFIG_CONTENT)
    
    # Configure the header template
    configure_file(
        "${CMAKE_SOURCE_DIR}/src/config/config.h.in"
        "${CMAKE_BINARY_DIR}/include/config.h"
        @ONLY
    )
    
    # Add include directory
    include_directories(PUBLIC "${CMAKE_BINARY_DIR}/include")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")

# add_compile_options(
#     ### Gnu/Clang C Options
#     $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
#     $<$<COMPILE_LANG_AND_ID:C,Clang>:-fcolor-diagnostics>

#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall>
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wextra>
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Werror>
#     $<$<COMPILE_LANG_AND_ID:C,Clang>:-Weverything>
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-unused-variable>
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-unused-parameter>
#     $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-type-limits>
# )
include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)


FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(PicoApp_Tests ${TEST_SOURCES} 
../src/StepperState.cxx 
../src/MachineState.cxx 
../src/Display.cxx
../src/Settings.cxx
)

add_compile_options(
    ### Gnu/Clang C Options
    $<$<CXX_COMPILER_ID:GNU>:-fdiagnostics-color=always>
    # $<$<CXX_COMPILER_ID:Clang>:-fcolor-diagnostics>
)
target_compile_definitions(PicoApp_Tests PRIVATE UNIT_TEST)
# Link against GoogleTest and GoogleMock
target_link_libraries(PicoApp_Tests
GTest::gtest_main
GTest::gmock_main
nlohmann_json::nlohmann_json
)

add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/tests/compile_commands.json
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/compile_commands.json
  ${CMAKE_SOURCE_DIR}/tests/compile_commands.json
  DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
  COMMENT "Copying compile_commands.json to tests root"
)

add_custom_target(copy_compile_commands_tests ALL
  DEPENDS ${CMAKE_SOURCE_DIR}/tests/compile_commands.json
)

add_dependencies(PicoApp_Tests copy_compile_commands_tests)


gtest_discover_tests(PicoApp_Tests)

