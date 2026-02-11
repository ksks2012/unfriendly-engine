cmake_minimum_required(VERSION 3.10)
project(RocketSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER_LAUNCHER ccache)
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(GTest REQUIRED)

# set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)

# path of spdlog 
set(SPDLOG_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/lib/spdlog/)
file(GLOB SPDLOG_SOURCES ${SPDLOG_INCLUDE_DIRS}/*.h)

# path of IMGUI
set(IMGUI_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/lib/imgui)
file(GLOB IMGUI_SOURCES ${IMGUI_INCLUDE_DIRS}/*.cpp)

# path of JSON
set(NLOHMANN_JSON_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/lib)

include_directories(
    ${SPDLOG_INCLUDE_DIRS}
    ${IMGUI_INCLUDE_DIRS}
    ${NLOHMANN_JSON_INCLUDE_DIRS}
)

# Collect source files
file(GLOB CORE_SOURCES src/core/*.cpp src/core/*/*.cpp)
file(GLOB RENDERING_SOURCES src/rendering/*.cpp src/rendering/*/*.cpp)
file(GLOB LOGGING_SOURCES src/logging/*.cpp src/logging/*/*.cpp)
file(GLOB APP_SOURCES src/app/*.cpp src/app/*/*.cpp)
file(GLOB UI_SOURCES src/ui/*.cpp src/ui/*/*.cpp)
set(MAIN_SOURCE src/main.cpp)

file(GLOB HEADERS include/*.h)

set(SOURCES
    ${CORE_SOURCES}
    ${RENDERING_SOURCES}
    ${LOGGING_SOURCES}
    ${APP_SOURCES}
    ${UI_SOURCES}
    ${MAIN_SOURCE}
    ${SPDLOG_SOURCES}
    ${IMGUI_SOURCES}
    ${NLOHMANN_JSON_SOURCES}
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    glfw
    GLEW::GLEW
)

# Test target
file(GLOB TEST_SOURCES tests/*.cpp)
set(TEST_SOURCES
    ${TEST_SOURCES}
    ${CORE_SOURCES}
    ${RENDERING_SOURCES}
    ${LOGGING_SOURCES}
    ${APP_SOURCES}
    ${UI_SOURCES}
    ${SPDLOG_SOURCES}
    ${IMGUI_SOURCES}
    ${NLOHMANN_JSON_SOURCES}
)
add_executable(runTests ${TEST_SOURCES} ${HEADERS})
target_link_libraries(runTests
    /usr/lib/x86_64-linux-gnu/libgtest.a
    /usr/lib/x86_64-linux-gnu/libgtest_main.a
    /usr/lib/x86_64-linux-gnu/libgmock.a
    /usr/lib/x86_64-linux-gnu/libgmock_main.a
    ${OPENGL_LIBRARIES}
    glfw
    GLEW::GLEW
    pthread
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(TARGETS runTests DESTINATION bin)