cmake_minimum_required(VERSION 3.10)
project(RocketSimulation)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(GTest REQUIRED)

# Main program source files
set(SOURCES
    src/main.cpp
    src/app.cpp
    src/simulation.cpp
    src/shader.cpp
    src/render_object.cpp
    src/rocket.cpp
    src/input_handler.cpp
    src/ui.cpp
    src/config.cpp
    src/flight_plan.cpp
    src/camera.cpp
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_impl_glfw.cpp
    lib/imgui/imgui_impl_opengl3.cpp
)

add_executable(RocketSimulation ${SOURCES})

target_include_directories(RocketSimulation PRIVATE
    include
    ${OPENGL_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    lib/imgui
    lib/
)

target_link_libraries(RocketSimulation
    ${OPENGL_LIBRARIES}
    glfw
    GLEW::GLEW
)

# Test program
enable_testing()
set(TEST_SOURCES
    tests/test_config.cpp
    tests/test_rocket.cpp
    src/config.cpp
    src/rocket.cpp
    src/input_handler.cpp
    src/simulation.cpp
    src/render_object.cpp
    src/shader.cpp
    src/flight_plan.cpp
)

add_executable(runTests ${TEST_SOURCES})

target_include_directories(runTests PRIVATE
    include
    ${OPENGL_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    lib/
    /usr/local/lib/gtest/include
    /usr/local/lib/gmock/include
    /usr/local/lib/gtest
    /usr/local/lib/gmock
)

target_link_libraries(runTests
    /usr/local/lib/libgtest.a
    /usr/local/lib/libgtest_main.a
    /usr/local/lib/libgmock.a
    /usr/local/lib/libgmock_main.a
    ${OPENGL_LIBRARIES}
    glfw
    GLEW::GLEW
    pthread
)

add_test(NAME UnitTests COMMAND runTests)