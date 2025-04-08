#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "simulation.h"

#include <GLFW/glfw3.h>
#include <unordered_map>

class InputHandler {
public:
InputHandler(GLFWwindow*, const Config&);

    void process(Simulation&);

private:
    GLFWwindow* window;
    std::unordered_map<int, double> lastPressTimes;
    float rotationSpeed;
    float directionCooldown;
};

#endif