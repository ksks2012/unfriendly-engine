#ifndef ROCKET_H
#define ROCKET_H

#include "shader.h"
#include "render_object.h"

#include <glm/glm.hpp>
#include <memory>


class Rocket {
private:
    struct State {
        glm::vec3 position;
        glm::vec3 velocity;
    };

    std::unique_ptr<RenderObject> renderObject;
    std::unique_ptr<RenderObject> trajectoryObject; // Trajectory rendering object
    std::vector<glm::vec3> trajectoryPoints;       // Trajectory points

    float mass;             // Total mass of the rocket (kg)
    float fuel_mass;        // Fuel mass (kg)
    float thrust;           // Thrust (N)
    float exhaust_velocity; // Exhaust velocity (m/s)
    glm::vec3 position;     // Position (m)
    glm::vec3 velocity;     // Velocity (m/s)
    float time;             // Time (s)
    bool launched;          // Whether the rocket is launched

    const float G = 6.674e-11f;     // Gravitational constant
    const float M = 5.972e24f;      // Earth's mass
    const float R_e = 6371000.0f;   // Earth's radius
    const float scale = 0.001f; // Consistent with Earth, 1 m = 0.001 units

    float trajectorySampleTime; // Trajectory sampling timer

public:
    Rocket();

    void init();
    void update(float);
    
    void render(const Shader&) const;
    void toggleLaunch();
    void resetTime();

    // Getter
    glm::vec3 getPosition() const;
    glm::vec3 getVelocity() const;
    float getTime() const;
    bool isLaunched() const;
    float getMass() const;
    float getFuelMass() const;
    float getThrust() const;
    float getExhaustVelocity() const;

    const std::vector<glm::vec3>& getTrajectoryPoints() const;

private:
    glm::vec3 computeAcceleration(const State&, float) const;
    void updateTrajectory();
    glm::vec3 offsetPosition() const;
};

#endif