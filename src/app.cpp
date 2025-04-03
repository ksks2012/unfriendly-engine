#include <stdexcept>

#include "app.h"

App::App(const std::string& title, int width, int height) : window(nullptr) {
    if (!glfwInit()) 
        throw std::runtime_error("Failed to initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLEW");
    }

    shader.init();
    simulation.init();
    inputHandler = std::make_unique<InputHandler>(window);
    ui = std::make_unique<UI>(window);
    glEnable(GL_DEPTH_TEST);
}

App::~App() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void App::run() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        inputHandler->process(simulation);

        simulation.update();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, static_cast<int>(height * 0.8f));
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        simulation.render(shader);
        ui->render(simulation.getTimeScale(), simulation.getRocket(), width, height);
        glfwSwapBuffers(window);
    }
}