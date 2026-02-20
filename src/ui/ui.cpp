#include "ui/ui.h"
#include "core/simulation.h"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>

UI::UI(GLFWwindow* win, Simulation& sim) 
    : window_(win), map_(sim), simulation_(sim), fpsCounter_(FPSCounter()), lastTime_(0.0f), selectedBody_("rocket") {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

UI::~UI() = default;

void UI::render(float timeScale, const Rocket& rocket, const Camera& camera, int width, int height) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Message box
    float sceneHeight = height * 0.8f;
    ImGui::SetNextWindowPos(ImVec2(10, sceneHeight + 10));
    ImGui::SetNextWindowSize(ImVec2(width - 20, height * 0.2f - 20));
    ImGui::Begin("Simulation Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    // Format time scale with appropriate suffix for large values
    if (timeScale >= 86400.0f) {
        ImGui::Text("Time Scale: %.1f days/s", timeScale / 86400.0f);
    } else if (timeScale >= 3600.0f) {
        ImGui::Text("Time Scale: %.1f hours/s", timeScale / 3600.0f);
    } else if (timeScale >= 60.0f) {
        ImGui::Text("Time Scale: %.1f min/s", timeScale / 60.0f);
    } else {
        ImGui::Text("Time Scale: %.1fx", timeScale);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(Q/E: adjust, Shift+Q/E: fast, R: reset)");
    ImGui::Text("Mass: %.1f kg", rocket.getMass());
    ImGui::Text("Fuel Mass: %.1f kg", rocket.getFuelMass());
    ImGui::Text("Thrust: %.1f N", rocket.getThrust());
    ImGui::Text("Exhaust Velocity: %.1f m/s", rocket.getExhaustVelocity());
    ImGui::Text("Position (Geocentric): %s", glm::to_string(glm::vec3(rocket.getPosition())).c_str());
    ImGui::Text("Velocity: %s", glm::to_string(glm::vec3(rocket.getVelocity())).c_str());
    ImGui::Text("Thrust Direction: %s", glm::to_string(glm::vec3(rocket.getThrustDirection())).c_str());
    ImGui::Text("Altitude: %.1f m", glm::length(rocket.getPosition()) - 6371000.0);
    ImGui::Text("Time: %.1f s", rocket.getTime());
    ImGui::Text("Launched: %s", rocket.isLaunched() ? "Yes" : "No");
    if (rocket.isCrashed()) {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "*** CRASHED ***");
    }
    ImGui::End();

    // Thumbnail (top-left corner) - Hidden
    // map_.render(width, height);

    renderFPS();
    renderCameraMode(camera, width, height);
    renderBodySelector(camera, width, height);
    
    // Render NavBall HUD (positioned at bottom-right of scene area)
    if (showNavBall_) {
        float sceneHeight = height * 0.8f;
        float navBallSize = 150.0f;
        float navBallX = width - navBallSize - 80;
        float navBallY = sceneHeight - navBallSize - 160;
        
        // Get Earth position for reference frame (convert to vec3 for rendering)
        const auto& bodies = simulation_.getBodies();
        glm::vec3 earthPos(0.0f);
        if (bodies.find("earth") != bodies.end()) {
            earthPos = glm::vec3(bodies.at("earth")->position);
        }
        
        navBall_.render(rocket, earthPos, navBallX, navBallY, navBallSize);
    }
    
    // Render Orbital Info panel (positioned below Camera Control, which is now at y=10, height=230)
    if (showOrbitalInfo_) {
        const auto& bodies = simulation_.getBodies();
        orbitalInfo_.render(rocket, bodies, 10.0f, 250.0f);
    }
    
    // Render planet labels (must be called after NewFrame and before Render)
    if (hasPendingLabelRender_) {
        renderPlanetLabelsInternal(camera, pendingProjection_, pendingView_, 
                                   pendingScale_, width, height);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UI::renderFPS() {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime_;
    lastTime_ = currentTime;
    fpsCounter_.update(deltaTime);

    int windowWidth, windowHeight;
    glfwGetWindowSize(window_, &windowWidth, &windowHeight);

    ImGui::SetNextWindowPos(ImVec2(windowWidth - 110.0f, 10.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(100.0f, 25.0f), ImGuiCond_Always);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("FPS Display", nullptr, windowFlags);
    ImGui::Text("FPS: %.1f", fpsCounter_.getFPS());
    ImGui::End();
}

void UI::renderCameraMode(const Camera& camera, int width, int height) {
    // Position at top-left (Map View is hidden)
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(220.0f, 230.0f), ImGuiCond_Always);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Camera Control", nullptr, windowFlags);
    ImGui::Text("Mode: %s", camera.getModeName());
    ImGui::Separator();
    ImGui::Text("Controls:");
    ImGui::Text("F - Free View");
    ImGui::Text("L - Follow Rocket");
    ImGui::Text("1 - Earth View");
    ImGui::Text("2 - Moon View");
    ImGui::Text("3 - Earth-Moon Overview");
    ImGui::Text("4 - Inner Solar System");
    ImGui::Text("5 - Full Solar System");
    ImGui::Separator();
    ImGui::Text("HUD Toggles:");
    ImGui::Text("P - Planet Labels");
    ImGui::Text("N - NavBall");
    ImGui::Text("O - Orbital Info");
    ImGui::End();
}

void UI::renderBodySelector(const Camera& camera, int width, int height) {
    // Position on the right side of the screen, below FPS display
    float panelWidth = 180.0f;
    float panelHeight = 350.0f;  // Increased height for reorganized layout
    ImGui::SetNextWindowPos(ImVec2(width - panelWidth - 10.0f, 45.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Celestial Bodies", nullptr, windowFlags);
    
    const auto& bodies = simulation_.getBodies();
    
    // Stars
    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.0f, 1.0f), "Star");
    ImGui::Separator();
    if (bodies.find("sun") != bodies.end()) {
        if (ImGui::Selectable("  Sun", selectedBody_ == "sun")) {
            selectedBody_ = "sun";
            if (bodySelectCallback_) bodySelectCallback_("sun");
        }
    }
    
    // Inner Planets with their satellites
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Inner Planets");
    ImGui::Separator();
    
    // Mercury
    if (bodies.find("mercury") != bodies.end()) {
        if (ImGui::Selectable("  Mercury", selectedBody_ == "mercury")) {
            selectedBody_ = "mercury";
            if (bodySelectCallback_) bodySelectCallback_("mercury");
        }
    }
    
    // Venus
    if (bodies.find("venus") != bodies.end()) {
        if (ImGui::Selectable("  Venus", selectedBody_ == "venus")) {
            selectedBody_ = "venus";
            if (bodySelectCallback_) bodySelectCallback_("venus");
        }
    }
    
    // Earth and its Moon
    if (bodies.find("earth") != bodies.end()) {
        if (ImGui::Selectable("  Earth", selectedBody_ == "earth")) {
            selectedBody_ = "earth";
            if (bodySelectCallback_) bodySelectCallback_("earth");
        }
        // Moon as sub-item under Earth
        if (bodies.find("moon") != bodies.end()) {
            if (ImGui::Selectable("    - Moon", selectedBody_ == "moon")) {
                selectedBody_ = "moon";
                if (bodySelectCallback_) bodySelectCallback_("moon");
            }
        }
    }
    
    // Mars
    if (bodies.find("mars") != bodies.end()) {
        if (ImGui::Selectable("  Mars", selectedBody_ == "mars")) {
            selectedBody_ = "mars";
            if (bodySelectCallback_) bodySelectCallback_("mars");
        }
    }
    
    // Outer Planets
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "Outer Planets");
    ImGui::Separator();
    
    const char* outerPlanets[] = {"jupiter", "saturn", "uranus", "neptune"};
    const char* outerPlanetNames[] = {"Jupiter", "Saturn", "Uranus", "Neptune"};
    for (int i = 0; i < 4; i++) {
        if (bodies.find(outerPlanets[i]) != bodies.end()) {
            std::string label = "  " + std::string(outerPlanetNames[i]);
            if (ImGui::Selectable(label.c_str(), selectedBody_ == outerPlanets[i])) {
                selectedBody_ = outerPlanets[i];
                if (bodySelectCallback_) bodySelectCallback_(outerPlanets[i]);
            }
        }
    }
    
    // Spacecraft
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "Spacecraft");
    ImGui::Separator();
    if (ImGui::Selectable("  Rocket", selectedBody_ == "rocket")) {
        selectedBody_ = "rocket";
        if (bodySelectCallback_) bodySelectCallback_("rocket");
    }
    
    ImGui::End();
}

glm::vec2 UI::worldToScreen(const glm::vec3& worldPos, const glm::mat4& projection, 
                             const glm::mat4& view, int width, int height) {
    // Scene height is 80% of window height (consistent with simulation render)
    float sceneHeight = height * 0.8f;
    
    // Transform world position to clip space
    glm::vec4 clipPos = projection * view * glm::vec4(worldPos, 1.0f);
    
    // Perspective division
    if (clipPos.w <= 0.0f) {
        return glm::vec2(-1000.0f, -1000.0f);  // Behind camera
    }
    
    glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;
    
    // Check if position is within visible frustum
    if (ndcPos.x < -1.0f || ndcPos.x > 1.0f || 
        ndcPos.y < -1.0f || ndcPos.y > 1.0f ||
        ndcPos.z < -1.0f || ndcPos.z > 1.0f) {
        return glm::vec2(-1000.0f, -1000.0f);  // Outside frustum
    }
    
    // Convert to screen coordinates (only using scene area, not the info panel at bottom)
    float screenX = (ndcPos.x + 1.0f) * 0.5f * width;
    float screenY = (1.0f - ndcPos.y) * 0.5f * sceneHeight;  // Y is inverted
    
    return glm::vec2(screenX, screenY);
}

void UI::renderPlanetLabels(const Camera& camera, const glm::mat4& projection, const glm::mat4& view,
                            float scale, int width, int height) {
    // Store pending render data - actual rendering happens in render() after ImGui::NewFrame()
    // Only enable if in appropriate camera mode and labels are visible
    if ((camera.mode == Camera::Mode::SolarSystem || 
         camera.mode == Camera::Mode::FullSolarSystem ||
         camera.mode == Camera::Mode::FocusBody) && showPlanetLabels_) {
        hasPendingLabelRender_ = true;
        pendingProjection_ = projection;
        pendingView_ = view;
        pendingScale_ = scale;
    } else {
        hasPendingLabelRender_ = false;
    }
}

void UI::renderPlanetLabelsInternal(const Camera& camera, const glm::mat4& projection, const glm::mat4& view,
                                     float scale, int width, int height) {
    const auto& bodies = simulation_.getBodies();
    
    // Define planet display names and colors
    std::map<std::string, std::pair<std::string, ImVec4>> planetInfo = {
        {"sun",     {"Sun",       ImVec4(1.0f, 0.8f, 0.0f, 1.0f)}},
        {"mercury", {"Mercury",   ImVec4(0.7f, 0.7f, 0.7f, 1.0f)}},
        {"venus",   {"Venus",     ImVec4(1.0f, 0.9f, 0.7f, 1.0f)}},
        {"earth",   {"Earth",     ImVec4(0.3f, 0.6f, 1.0f, 1.0f)}},
        {"moon",    {"Moon",      ImVec4(0.8f, 0.8f, 0.8f, 1.0f)}},
        {"mars",    {"Mars",      ImVec4(1.0f, 0.4f, 0.2f, 1.0f)}},
        {"jupiter", {"Jupiter",   ImVec4(0.9f, 0.7f, 0.5f, 1.0f)}},
        {"saturn",  {"Saturn",    ImVec4(0.9f, 0.8f, 0.5f, 1.0f)}},
        {"uranus",  {"Uranus",    ImVec4(0.5f, 0.8f, 0.9f, 1.0f)}},
        {"neptune", {"Neptune",   ImVec4(0.3f, 0.4f, 0.9f, 1.0f)}}
    };
    
    // Determine which planets to label based on view mode
    std::vector<std::string> planetsToLabel;
    if (camera.mode == Camera::Mode::SolarSystem) {
        planetsToLabel = {"sun", "mercury", "venus", "earth", "mars"};
    } else if (camera.mode == Camera::Mode::FullSolarSystem) {
        planetsToLabel = {"sun", "mercury", "venus", "earth", "mars", 
                          "jupiter", "saturn", "uranus", "neptune"};
    } else if (camera.mode == Camera::Mode::FocusBody) {
        // Show label for focused body and nearby objects
        planetsToLabel = {camera.focusBodyName};
        // Also show moon if focusing on earth
        if (camera.focusBodyName == "earth") {
            planetsToLabel.push_back("moon");
        }
    }
    
    // Draw labels for each visible planet
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    
    for (const auto& name : planetsToLabel) {
        auto it = bodies.find(name);
        if (it == bodies.end()) continue;
        
        glm::vec3 worldPos = glm::vec3((it->second->position - simulation_.getRenderOrigin()) * static_cast<double>(scale));
        glm::vec2 screenPos = worldToScreen(worldPos, projection, view, width, height);
        
        // Check if on screen
        if (screenPos.x < 0 || screenPos.x > width || 
            screenPos.y < 0 || screenPos.y > height * 0.8f) {
            continue;
        }
        
        auto& info = planetInfo[name];
        std::string label = info.first;
        ImVec4 color = info.second;
        
        // Draw label with background for better visibility
        ImVec2 textPos(screenPos.x + 10.0f, screenPos.y - 8.0f);
        ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
        
        // Background rectangle
        ImVec2 bgMin(textPos.x - 2.0f, textPos.y - 2.0f);
        ImVec2 bgMax(textPos.x + textSize.x + 2.0f, textPos.y + textSize.y + 2.0f);
        drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, 180), 3.0f);
        
        // Draw label text
        drawList->AddText(textPos, ImGui::ColorConvertFloat4ToU32(color), label.c_str());
        
        // Draw a small marker at the planet position
        drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), 4.0f, 
                                   ImGui::ColorConvertFloat4ToU32(color));
        drawList->AddCircle(ImVec2(screenPos.x, screenPos.y), 6.0f, 
                            IM_COL32(255, 255, 255, 200), 0, 1.5f);
    }
    
    // Reset pending flag
    hasPendingLabelRender_ = false;
}
