#ifndef UI_H
#define UI_H

#include "app/map.h"
#include "core/rocket.h"
#include "rendering/camera.h"
#include "ui/fps_counter.h"
#include "ui/navball.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
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
    
    // Set pending planet label render data (called before render())
    void renderPlanetLabels(const Camera& camera, const glm::mat4& projection, const glm::mat4& view, 
                            float scale, int width, int height);
    
    // Set callback for when a body is selected
    void setBodySelectCallback(BodySelectCallback callback) { bodySelectCallback_ = callback; }
    
    // Get currently selected body name
    const std::string& getSelectedBody() const { return selectedBody_; }
    
    // Toggle planet labels visibility
    void togglePlanetLabels() { showPlanetLabels_ = !showPlanetLabels_; }
    bool arePlanetLabelsVisible() const { return showPlanetLabels_; }
    
    // Toggle NavBall visibility
    void toggleNavBall() { showNavBall_ = !showNavBall_; }
    bool isNavBallVisible() const { return showNavBall_; }

private:
    GLFWwindow* window_;
    Map map_;
    Simulation& simulation_;  // Reference to simulation for accessing bodies
    FPSCounter fpsCounter_;
    float lastTime_;
    std::string selectedBody_;  // Currently selected body
    BodySelectCallback bodySelectCallback_;
    bool showPlanetLabels_ = true;  // Show planet labels in solar system view
    bool showNavBall_ = true;       // Show NavBall HUD
    NavBall navBall_;               // NavBall instance
    
    // Pending planet label render data
    bool hasPendingLabelRender_ = false;
    glm::mat4 pendingProjection_;
    glm::mat4 pendingView_;
    float pendingScale_ = 0.0f;
    
    // Helper function to convert 3D world position to 2D screen position
    glm::vec2 worldToScreen(const glm::vec3& worldPos, const glm::mat4& projection, 
                            const glm::mat4& view, int width, int height);
    
    // Internal function to actually render planet labels (called within ImGui frame)
    void renderPlanetLabelsInternal(const Camera& camera, const glm::mat4& projection, 
                                    const glm::mat4& view, float scale, int width, int height);
};
#endif