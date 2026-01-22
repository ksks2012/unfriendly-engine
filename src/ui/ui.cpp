#include "ui/ui.h"
#include <glm/gtx/string_cast.hpp>

UI::UI(GLFWwindow* win, Simulation& sim) : window_(win), map_(sim), fpsCounter_(FPSCounter()), lastTime_(0.0f) {
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
    ImGui::Text("Time Scale: %.1f", timeScale);
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
