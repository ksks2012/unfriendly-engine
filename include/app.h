#ifndef APP_H
#define APP_H

#include "input_handler.h"
#include "logger.h"
#include "simulation.h"
#include "shader.h"
#include "map.h"
#include "ui.h"

#include <GLFW/glfw3.h>
#include <memory>

class App {
public:
    App(const std::string& title, int width, int height, Config& config, std::shared_ptr<ILogger> logger);
    ~App();
    void run();

private:
    GLFWwindow* window;
    Config& config;
    Simulation simulation;
    Shader shader;
    std::unique_ptr<InputHandler> inputHandler;
    std::unique_ptr<UI> ui;
    std::unique_ptr<Map> map;
};

#endif