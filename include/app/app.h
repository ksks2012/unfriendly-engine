#ifndef APP_H
#define APP_H

#include "ui/input_handler.h"
#include "logging/logger.h"
#include "core/simulation.h"
#include "rendering/shader.h"
#include "app/map.h"
#include "ui/ui.h"

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