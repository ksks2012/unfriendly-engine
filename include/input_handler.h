#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "simulation.h"

#include <GLFW/glfw3.h>

class InputHandler {
public:
    InputHandler(GLFWwindow*);

    void process(Simulation&);

private:
    GLFWwindow* window;
};

#endif