#include "ui/ui.h"
#include "core/simulation.h"
#include <glm/gtx/string_cast.hpp>

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
    ImGui::Text("Position (Geocentric): %s", glm::to_string(rocket.getPosition()).c_str());
    ImGui::Text("Velocity: %s", glm::to_string(rocket.getVelocity()).c_str());
    ImGui::Text("Thrust Direction: %s", glm::to_string(rocket.getThrustDirection()).c_str());
    ImGui::Text("Altitude: %.1f m", glm::length(rocket.getPosition()) - 6371000.0f);
    ImGui::Text("Time: %.1f s", rocket.getTime());
    ImGui::Text("Launched: %s", rocket.isLaunched() ? "Yes" : "No");
    ImGui::End();

    // Thumbnail (top-left corner)
    map_.render(width, height);

    renderFPS();
    renderCameraMode(camera, width, height);
    renderBodySelector(camera, width, height);

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
    // Position below the Map View panel (Map is at y=10, height=200)
    ImGui::SetNextWindowPos(ImVec2(10.0f, 220.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(200.0f, 130.0f), ImGuiCond_Always);

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
    ImGui::Text("3 - System Overview");
    ImGui::End();
}

void UI::renderBodySelector(const Camera& camera, int width, int height) {
    // Position on the right side of the screen
    float panelWidth = 180.0f;
    float panelHeight = 350.0f;  // Increased height for reorganized layout
    ImGui::SetNextWindowPos(ImVec2(width - panelWidth - 10.0f, 10.0f), ImGuiCond_Always);
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
