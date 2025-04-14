#ifndef UI_H
#define UI_H

#include "rocket.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include "rocket.h"
#include "map.h"

class UI {
public:
    UI(GLFWwindow* win, Simulation& sim);
    ~UI();
    void render(float timeScale, const Rocket& rocket, int width, int height);
    void shutdown();

private:
    GLFWwindow* window;
    Map map; // 內嵌 Map 實例
};
#endif