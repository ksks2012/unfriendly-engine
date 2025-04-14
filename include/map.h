#ifndef MAP_H
#define MAP_H

#include "simulation.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Map {
public:
    Map(Simulation& sim);
    void render(int width, int height);

private:
    Simulation& simulation;
    bool mapClicked;
    glm::vec2 lastClickPos;
};

#endif