#ifndef UI_H
#define UI_H

#include "app/map.h"
#include "core/rocket.h"
#include "ui/fps_counter.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

class UI {
public:
    UI(GLFWwindow* win, Simulation& sim);
    ~UI();
    void render(float timeScale, const Rocket& rocket, int width, int height);
    void shutdown();
    void renderFPS();

private:
    GLFWwindow* window_;
    Map map_;
    FPSCounter fpsCounter_;
    float lastTime_;
};
#endif