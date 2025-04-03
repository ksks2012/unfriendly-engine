#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"
#include "render_object.h"
#include "simulation.h"
#include "input_handler.h"
#include "ui.h"


int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Rocket Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Initialize components
    Shader shader;
    Simulation sim;
    InputHandler input(window);
    UI ui(window);

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Handle input
        input.process(sim);

        // Update simulation (currently just state, later physics)
        sim.update();

        // Render
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, static_cast<int>(height * 0.8f));

        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        sim.render(shader);

        // UI
        ui.render(sim.getTimeScale(), width, height);

        glfwSwapBuffers(window);
    }

    // Cleanup
    ui.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}