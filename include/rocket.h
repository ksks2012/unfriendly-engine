#ifndef ROCKET_H
#define ROCKET_H

#include "config.h"
#include "shader.h"
#include "render_object.h"

#include <glm/glm.hpp>
#include <array>
#include <memory>


class Rocket {
private:
    struct State {
        glm::vec3 position;
        glm::vec3 velocity;
    };

    const float G;     // Gravitational constant
    const float M;      // Earth's mass
    const float R_e;   // Earth's radius
    const float scale;     // Consistent with Earth, 1 m = 0.001 units
    const float rho_0;         // Air density at sea level (kg/m^3)
    const float H;             // Scale height (m)
    const float Cd;            // Drag coefficient
    const float A;             // Cross-sectional area (m^2)
    float predictionDuration, predictionStep; // Prediction parameters

    std::unique_ptr<RenderObject> renderObject; // Rocket rendering object

    float mass;                // Total mass of the rocket (kg)
    float fuel_mass;           // Fuel mass (kg)
    float thrust;              // Thrust (N)
    float exhaust_velocity;    // Exhaust velocity (m/s)
    glm::vec3 position;        // Position (m)
    glm::vec3 velocity;        // Velocity (m/s)
    float time;                // Time (s)
    glm::vec3 thrustDirection; // Thrust direction
    bool launched;             // Whether the rocket is launched
    
    static constexpr size_t TRAJECTORY_SIZE = 1000;
    std::unique_ptr<RenderObject> trajectoryObject; // Trajectory rendering object
    std::array<glm::vec3, TRAJECTORY_SIZE> trajectoryPoints; // Trajectory points
    size_t trajectoryHead{0}; // Head of the trajectory
    size_t trajectoryCount{0}; // Number of trajectory points
    float trajectorySampleTime; // Trajectory sampling timer

    static constexpr size_t PREDICTION_SIZE = 100;
    std::unique_ptr<RenderObject> predictionObject; // Prediction rendering object
    std::vector<glm::vec3> predictionPoints; // Prediction points

public:
    Rocket(const Config&);

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
    glm::vec3 getThrustDirection() const;
    void setThrustDirection(const glm::vec3&);

    void predictTrajectory(float, float);

private:
    glm::vec3 computeAcceleration(const State&, float) const;
    void updateTrajectory();
    glm::vec3 offsetPosition() const;
    glm::vec3 offsetPosition(glm::vec3) const;
    State updateState(const State& state, float deltaTime, float& currentMass, float& currentFuel) const;
};

#endif