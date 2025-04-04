#ifndef SIMULATION_H
#define SIMULATION_H

#include "render_object.h"
#include "rocket.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <memory>

class Simulation {
public:
    Simulation();
    ~Simulation();

    void init();
    void update(float deltaTime);
    void render(const Shader&) const;

    void setTimeScale(float);
    void adjustTimeScale(float);
    void adjustCameraDistance(float);
    float getTimeScale() const;
    Rocket& getRocket();

private:
    std::unique_ptr<RenderObject> ground;
    Rocket rocket;
    float cameraDistance;
    float timeScale;
};

#endif