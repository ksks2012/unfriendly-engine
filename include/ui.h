#ifndef UI_H
#define UI_H

#include "rocket.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

class UI {
public:
    UI(GLFWwindow*);
    ~UI();

    void render(float, const Rocket &, int, int);
    void shutdown();
    
private:
    GLFWwindow* window;
};

#endif