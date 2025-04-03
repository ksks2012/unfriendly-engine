cmake_minimum_required(VERSION 3.10)
project(RocketSimulation)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/app.cpp
    src/simulation.cpp
    src/shader.cpp
    src/render_object.cpp
    src/input_handler.cpp
    src/ui.cpp
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
)

target_link_libraries(RocketSimulation
    ${OPENGL_LIBRARIES}
    glfw
    GLEW::GLEW
)