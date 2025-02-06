
set(CMAKE_BUILD_TYPE Debug) 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fno-omit-frame-pointer")


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add test executable
file(GLOB_RECURSE TEST_SOURCES "*.cpp")
add_executable(PicoApp_Tests ${TEST_SOURCES} ../src/StepperState.cxx ../src/MachineState.cxx ../src/Display.cxx)

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
