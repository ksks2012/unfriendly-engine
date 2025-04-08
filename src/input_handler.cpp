#include "input_handler.h"

#include <glm/gtc/matrix_transform.hpp>

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

    float rotationSpeed = 90.0f; // Rotation angle per second (degrees)
    double directionCooldown = 0.01; // Direction adjustment cooldown time
    if (isKeyPressedWithCooldown(GLFW_KEY_A, directionCooldown)) {
        Rocket& rocket = sim.getRocket();
        glm::vec3 currentDir = rocket.getThrustDirection(); // Assuming this getter is added, otherwise access directly
        glm::mat4 rotation = glm::rotate(
            glm::mat4(1.0f),
            glm::radians(static_cast<float>(rotationSpeed * directionCooldown)),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );        
        glm::vec3 newDir = glm::vec3(rotation * glm::vec4(currentDir, 0.0f));
        rocket.setThrustDirection(newDir);
    }
    if (isKeyPressedWithCooldown(GLFW_KEY_D, directionCooldown)) {
        Rocket& rocket = sim.getRocket();
        glm::vec3 currentDir = rocket.getThrustDirection();
        glm::mat4 rotation = glm::rotate(
            glm::mat4(1.0f),
            glm::radians(static_cast<float>(-rotationSpeed * directionCooldown)),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        glm::vec3 newDir = glm::vec3(rotation * glm::vec4(currentDir, 0.0f));
        rocket.setThrustDirection(newDir);
    }
}
