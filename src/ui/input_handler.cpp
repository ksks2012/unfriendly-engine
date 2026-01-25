#include "ui/input_handler.h"
#include "core/simulation.h"

InputHandler::InputHandler(GLFWwindow* win, Simulation& sim, const Config& config) 
    : window(win), simulation(sim), rotationSpeed(config.rocket_rotation_speed), 
      directionCooldown(config.rocket_direction_cooldown), lastX(400.0f), lastY(300.0f), 
      firstMouse(true), mouseSensitivity(0.1f) {
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xpos, double ypos) {
        static_cast<InputHandler*>(glfwGetWindowUserPointer(w))->mouseCallback(xpos, ypos);
    });
    glfwSetScrollCallback(window, [](GLFWwindow* w, double xoffset, double yoffset) {
        static_cast<InputHandler*>(glfwGetWindowUserPointer(w))->scrollCallback(yoffset);
    });
}

void InputHandler::process(Simulation& sim) {
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
    
    if (isKeyPressedWithCooldown(GLFW_KEY_A, directionCooldown)) {
        Rocket& rocket = sim.getRocket();
        glm::vec3 currentDir = rocket.getThrustDirection();
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

    // Camera mode switching
    // F - Free mode
    if (isKeyPressedWithCooldown(GLFW_KEY_F, 0.2)) {
        sim.adjustCameraMode(Camera::Mode::Free);
    }
    // L - Locked mode (follow rocket)
    if (isKeyPressedWithCooldown(GLFW_KEY_L, 0.2)) {
        sim.adjustCameraMode(Camera::Mode::Locked);
    }
    // 1 - Fixed on Earth
    if (isKeyPressedWithCooldown(GLFW_KEY_1, 0.2)) {
        sim.adjustCameraMode(Camera::Mode::FixedEarth);
    }
    // 2 - Fixed on Moon
    if (isKeyPressedWithCooldown(GLFW_KEY_2, 0.2)) {
        sim.adjustCameraMode(Camera::Mode::FixedMoon);
    }
    // 3 - Overview mode (Earth-Moon system)
    if (isKeyPressedWithCooldown(GLFW_KEY_3, 0.2)) {
        sim.adjustCameraMode(Camera::Mode::Overview);
    }
    // 4 - Solar System view
    if (isKeyPressedWithCooldown(GLFW_KEY_4, 0.2)) {
        sim.adjustCameraMode(Camera::Mode::SolarSystem);
    }
}

void InputHandler::mouseCallback(double xpos, double ypos) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        float deltaX = static_cast<float>(xpos - lastX);
        float deltaY = static_cast<float>(lastY - ypos);
        lastX = xpos;
        lastY = ypos;

        float deltaPitch = deltaY * mouseSensitivity;
        float deltaYaw = deltaX * mouseSensitivity;
        simulation.adjustCameraRotation(deltaPitch, deltaYaw);
    } else {
        firstMouse = true;
    }
}

void InputHandler::scrollCallback(double yoffset) {
    float deltaDistance = static_cast<float>(-yoffset * 1000.0f);
    simulation.adjustCameraDistance(deltaDistance);
}