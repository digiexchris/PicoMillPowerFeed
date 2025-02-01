
set(CMAKE_BUILD_TYPE Debug) 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fno-omit-frame-pointer")

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
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include the directories
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/lib/pico-ssd1306)

# Add test executable
file(GLOB_RECURSE TEST_SOURCES "*.cpp")
add_executable(PicoApp_Tests ${TEST_SOURCES} ../src/StepperState.cpp ../src/MachineState.cpp ../src/Display.cpp)

# add_subdirectory(${CMAKE_HOME_DIRECTORY}/lib/Simple3dStepperMovementPlanner StepTimingPlanner)

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
# StepTimingPlanner
# tcmalloc
# profiler
)

add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/tests/compile_commands.json
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/compile_commands.json
  ${CMAKE_SOURCE_DIR}/tests/compile_commands.json
  DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
  COMMENT "Copying compile_commands.json to tests root"
)

# Create a custom target that always runs the above command
add_custom_target(copy_compile_commands_tests ALL
  DEPENDS ${CMAKE_SOURCE_DIR}/tests/compile_commands.json
)

add_dependencies(PicoApp_Tests copy_compile_commands_tests)

include(GoogleTest)
gtest_discover_tests(PicoApp_Tests)

# set_tests_properties(StepTimingPlanner_Tests PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
