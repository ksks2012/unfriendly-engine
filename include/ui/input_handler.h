#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "app/config.h"
#include "core/simulation.h"

#include <GLFW/glfw3.h>
#include <unordered_map>

class InputHandler {
public:
    InputHandler(GLFWwindow* win, Simulation& sim, const Config& config);
    void process(Simulation& sim);

    void mouseCallback(double xpos, double ypos);
    void scrollCallback(double yoffset);

private:
    GLFWwindow* window;
    Simulation& simulation;
    double rotationSpeed;
    double directionCooldown;
    std::unordered_map<int, double> lastPressTimes;

    float lastX, lastY;
    bool firstMouse;
    float mouseSensitivity;
};

#endif