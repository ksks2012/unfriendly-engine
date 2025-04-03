#ifndef APP_H
#define APP_H

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "simulation.h"
#include "input_handler.h"
#include "ui.h"

class App {
public:
    App(const std::string& title, int width, int height);
    ~App();
    void run();

private:
    GLFWwindow* window;
    Shader shader;
    Simulation simulation;
    std::unique_ptr<InputHandler> inputHandler;
    std::unique_ptr<UI> ui;
};

#endif