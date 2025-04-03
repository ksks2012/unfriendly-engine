#ifndef UI_H
#define UI_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

class UI {
public:
    UI(GLFWwindow* window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    ~UI() = default;

    void render(float timeScale, int width, int height) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        float sceneHeight = height * 0.8f;
        ImGui::SetNextWindowPos(ImVec2(10, sceneHeight + 10));
        ImGui::SetNextWindowSize(ImVec2(width - 20, height * 0.2f - 20));
        ImGui::Begin("Simulation Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("Time Scale: %.1f", timeScale);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
};

#endif