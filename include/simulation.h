#ifndef SIMULATION_H
#define SIMULATION_H

#include "body.h"
#include "flight_plan.h"
#include "render_object.h"
#include "rocket.h"
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <memory>
#include <string>

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

    glm::vec3 getMoonPos() const;
    const std::unordered_map<std::string, Body>& getBodies() const;

private:
    Config config;
    Rocket rocket;
    // std::unique_ptr<RenderObject> earth;
    // std::unique_ptr<RenderObject> moon;
    std::unordered_map<std::string, Body> bodies;
    float R_e; // Earth's radius
    float R_moon;

    // Camera parameters
    float cameraDistance;
    float cameraPitch;  // Pitch angle (degrees)
    float cameraYaw;    // Yaw angle (degrees)
    float timeScale;

    void updateCameraPosition() const; // Update camera position

    glm::vec3 moonPos;
    float moonAngularSpeed;
    float moonMass;
    std::unique_ptr<RenderObject> mapEarth;
    std::unique_ptr<RenderObject> mapMoon;
    std::unique_ptr<RenderObject> mapRocket;
};

#endif