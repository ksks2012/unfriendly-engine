#ifndef SIMULATION_H
#define SIMULATION_H

#include "flight_plan.h"
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
    explicit Simulation(Config& config);
    ~Simulation();

    void init();
    void update(float deltaTime);
    void render(const Shader& shader) const;

    void setTimeScale(float ts);
    void adjustTimeScale(float delta);
    void adjustCameraDistance(float delta);
    void adjustCameraRotation(float deltaPitch, float deltaYaw); // Adjust camera rotation

    float getTimeScale() const;
    Rocket& getRocket();

private:
    Config config;
    Rocket rocket;
    std::unique_ptr<RenderObject> earth;
    float R_e; // Earth's radius

    // Camera parameters
    float cameraDistance;
    float cameraPitch;  // Pitch angle (degrees)
    float cameraYaw;    // Yaw angle (degrees)
    float timeScale;

    void updateCameraPosition() const; // Update camera position
};

#endif