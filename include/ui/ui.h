#ifndef UI_H
#define UI_H

#include "core/rocket.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include "core/rocket.h"
#include "app/map.h"

class UI {
public:
    UI(GLFWwindow* win, Simulation& sim);
    ~UI();
    void render(float timeScale, const Rocket& rocket, int width, int height);
    void shutdown();

private:
    GLFWwindow* window;
    Map map;
};
#endif