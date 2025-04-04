#ifndef ROCKET_H
#define ROCKET_H

#include "shader.h"
#include "render_object.h"

#include <glm/glm.hpp>
#include <memory>

class Rocket {
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

private:
    std::unique_ptr<RenderObject> renderObject;
    float mass;             // Total mass of the rocket (kg)
    float fuel_mass;        // Fuel mass (kg)
    float thrust;           // Thrust (N)
    float exhaust_velocity; // Exhaust velocity (m/s)
    glm::vec3 position;     // Position (m)
    glm::vec3 velocity;     // Velocity (m/s)
    float time;             // Time (s)
    bool launched;          // Whether the rocket is launched
};

#endif