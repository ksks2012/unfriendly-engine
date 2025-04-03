#include "input_handler.h"

InputHandler::InputHandler(GLFWwindow* win) : window(win) {}

void InputHandler::process(Simulation& sim) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        sim.adjustTimeScale(0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        sim.adjustTimeScale(-0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        sim.adjustCameraDistance(-10.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        sim.adjustCameraDistance(10.0f);
    }
}
