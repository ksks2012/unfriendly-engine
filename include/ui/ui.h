#ifndef UI_H
#define UI_H

#include "app/map.h"
#include "core/rocket.h"
#include "rendering/camera.h"
#include "ui/fps_counter.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

class UI {
public:
    // Callback type for body selection
    using BodySelectCallback = std::function<void(const std::string&)>;

    UI(GLFWwindow* win, Simulation& sim);
    ~UI();
    void render(float timeScale, const Rocket& rocket, const Camera& camera, int width, int height);
    void shutdown();
    void renderFPS();
    void renderCameraMode(const Camera& camera, int width, int height);
    void renderBodySelector(const Camera& camera, int width, int height);
    
    // Set callback for when a body is selected
    void setBodySelectCallback(BodySelectCallback callback) { bodySelectCallback_ = callback; }
    
    // Get currently selected body name
    const std::string& getSelectedBody() const { return selectedBody_; }

private:
    GLFWwindow* window_;
    Map map_;
    Simulation& simulation_;  // Reference to simulation for accessing bodies
    FPSCounter fpsCounter_;
    float lastTime_;
    std::string selectedBody_;  // Currently selected body
    BodySelectCallback bodySelectCallback_;
};
#endif