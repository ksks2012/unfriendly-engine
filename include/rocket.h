#ifndef ROCKET_H
#define ROCKET_H

#include "config.h"
#include "flight_plan.h"
#include "shader.h"
#include "render_object.h"

#include <glm/glm.hpp>
#include <gtest/gtest_prod.h>
#include <array>
#include <memory>
#include <vector>

class Rocket {
private:
    // Variables
    struct State {
        glm::vec3 position;
        glm::vec3 velocity;
    };

    const Config& config;    
    std::unique_ptr<IRenderObject> renderObject; // Rocket rendering object

    float predictionDuration, predictionStep; // Prediction parameters

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
    std::unique_ptr<IRenderObject> trajectoryObject; // Trajectory rendering object
    std::array<glm::vec3, TRAJECTORY_SIZE> trajectoryPoints; // Trajectory points
    size_t trajectoryHead{0}; // Head of the trajectory
    size_t trajectoryCount{0}; // Number of trajectory points
    float trajectorySampleTime; // Trajectory sampling timer

    static constexpr size_t PREDICTION_SIZE = 100;
    std::unique_ptr<IRenderObject> predictionObject; // Prediction rendering object
    std::vector<glm::vec3> predictionPoints; // Prediction points

    FlightPlan flightPlan;

    // For testing
    FRIEND_TEST(RocketTest, InitInjectsMockRenderObject);
    FRIEND_TEST(RocketTest, Initialization);
    FRIEND_TEST(RocketTest, OffsetPosition_Default);
    FRIEND_TEST(RocketTest, OffsetPosition_CustomPosition);
    FRIEND_TEST(RocketTest, FlightPlanExecution);

    void setRenderObjects(std::unique_ptr<IRenderObject> render,
        std::unique_ptr<IRenderObject> trajectory,
        std::unique_ptr<IRenderObject> prediction);

    // Private functions
    glm::vec3 computeAcceleration(const State&, float) const;
    void updateTrajectory();
    glm::vec3 offsetPosition() const;
    glm::vec3 offsetPosition(glm::vec3) const;
    State updateState(const State& state, float deltaTime, float& currentMass, float& currentFuel) const;

public:
    Rocket(const Config&, const FlightPlan&);

    // Public functions
    void init();
    void update(float);
    void render(const Shader&) const;
    void toggleLaunch();
    void resetTime();

    // Getters
    glm::vec3 getPosition() const;
    glm::vec3 getVelocity() const;
    float getTime() const;
    bool isLaunched() const;
    float getMass() const;
    float getFuelMass() const;
    float getThrust() const;
    float getExhaustVelocity() const;
    glm::vec3 getThrustDirection() const;

    // Setter
    void setThrustDirection(const glm::vec3&);

    // Prediction
    void predictTrajectory(float, float);
};

#endif