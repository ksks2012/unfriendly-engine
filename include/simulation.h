#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "render_object.h"
#include "shader.h"

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Simulation {
public:
    Simulation();

    void init();
    void update();
    void render(const Shader&) const;

    void setTimeScale(float);
    void adjustTimeScale(float);
    void adjustCameraDistance(float);
    float getTimeScale() const;

private:
    std::unique_ptr<RenderObject> rocket, ground;
    glm::vec3 rocketPos;
    float cameraDistance;
    float timeScale;
};

#endif
