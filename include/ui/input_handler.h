#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "app/config.h"
#include "core/simulation.h"

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <functional>

class InputHandler {
public:
    // Callback types for UI actions
    using ToggleCallback = std::function<void()>;

    InputHandler(GLFWwindow* win, Simulation& sim, const Config& config);
    void process(Simulation& sim);

    void mouseCallback(double xpos, double ypos);
    void scrollCallback(double yoffset);
    
    // Set callbacks for UI toggles
    void setTogglePlanetLabelsCallback(ToggleCallback callback) { 
        togglePlanetLabelsCallback_ = callback; 
    }
    void setToggleNavBallCallback(ToggleCallback callback) {
        toggleNavBallCallback_ = callback;
    }
    void setToggleOrbitalInfoCallback(ToggleCallback callback) {
        toggleOrbitalInfoCallback_ = callback;
    }

private:
    GLFWwindow* window;
    Simulation& simulation;
    double rotationSpeed;
    double directionCooldown;
    std::unordered_map<int, double> lastPressTimes;

    float lastX, lastY;
    bool firstMouse;
    float mouseSensitivity;
    
    ToggleCallback togglePlanetLabelsCallback_;
    ToggleCallback toggleNavBallCallback_;
    ToggleCallback toggleOrbitalInfoCallback_;
};

#endif