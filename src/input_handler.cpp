#include "input_handler.h"


InputHandler::InputHandler(GLFWwindow* win) : window(win) {}

void InputHandler::process(Simulation& sim) {
    // Set up a cooldown for key presses
    double currentTime = glfwGetTime();
    auto isKeyPressedWithCooldown = [&](int key, double cooldown) {
        if (glfwGetKey(window, key) == GLFW_PRESS) {
            if (currentTime - lastPressTimes[key] > cooldown) {
                lastPressTimes[key] = currentTime;
                return true;
            }
        }
        return false;
    };

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (isKeyPressedWithCooldown(GLFW_KEY_Q, 0.05)) {
        sim.adjustTimeScale(0.1f);
    }
    if (isKeyPressedWithCooldown(GLFW_KEY_E, 0.05)) {
        sim.adjustTimeScale(-0.1f);
    }
    if (isKeyPressedWithCooldown(GLFW_KEY_W, 0.01)) {
        sim.adjustCameraDistance(-100.0f);
    }
    if (isKeyPressedWithCooldown(GLFW_KEY_S, 0.01)) {
        sim.adjustCameraDistance(100.0f);
    }
    if (isKeyPressedWithCooldown(GLFW_KEY_SPACE, 0.2)) {
        sim.getRocket().toggleLaunch();
    }
}
