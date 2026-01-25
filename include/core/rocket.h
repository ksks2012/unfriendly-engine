#ifndef ROCKET_H
#define ROCKET_H

#define GLM_ENABLE_EXPERIMENTAL

#include "body.h"
#include "app/config.h"
#include "core/flight_plan.h"
#include "logging/logger.h"
#include "rendering/shader.h"
#include "rendering/render_object.h"
#include "rendering/trajectory.h"
#include "rendering/trajectory_factory.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <gtest/gtest_prod.h>
#include <array>
#include <memory>
#include <vector>
#include <unordered_map>

class Rocket : Body {
private:
    float predictionDuration, predictionStep; // Prediction parameters
    float predictionTimer_ = 0.0f;            // Timer for prediction update frequency
    float predictionUpdateInterval_ = 0.5f;   // Update prediction every 0.5 seconds

    float fuel_mass;           // Fuel mass (kg)
    float thrust;              // Thrust (N)
    float exhaust_velocity;    // Exhaust velocity (m/s)
    float time;                // Time (s)
    glm::vec3 thrustDirection; // Thrust direction
    bool launched;             // Whether the rocket is launched
    
    FlightPlan flightPlan;

    // For testing
    FRIEND_TEST(RocketTest, InitInjectsMockRenderObject);
    FRIEND_TEST(RocketTest, Initialization);
    FRIEND_TEST(RocketTest, OffsetPosition_Default);
    FRIEND_TEST(RocketTest, OffsetPosition_CustomPosition);
    FRIEND_TEST(RocketTest, FlightPlanExecution);
    FRIEND_TEST(RocketTest, ConcurrentUpdate);

    void setRender(std::unique_ptr<IRenderObject> render);
    void setTrajectoryRender(std::unique_ptr<IRenderObject> trajectory, std::unique_ptr<IRenderObject> prediction);

    // Private functions
    // Runge-Kutta 4th order method
    glm::vec3 computeAccelerationRK4(float currentMass, const BODY_MAP& bodies) const;
    void updateTrajectory();
    glm::vec3 offsetPosition() const;
    glm::vec3 offsetPosition(glm::vec3) const;
    Body updateStateRK4(const Body& state, float deltaTime, float& currentMass, float& currentFuel, const BODY_MAP& bodies) const;

public:
    Rocket(const Config&, std::shared_ptr<ILogger> logger, const FlightPlan&);

    // Public functions
    void init();
    void update(float, const BODY_MAP&);
    void render(const Shader&) const;
    void toggleLaunch();
    void resetTime();

    // Getters
    glm::vec3 getPosition() const;
    glm::vec3 getVelocity() const;
    glm::vec3 getRenderPosition() const;  // Get position in rendering coordinates
    float getTime() const;
    bool isLaunched() const;
    float getMass() const;
    float getFuelMass() const;
    float getThrust() const;
    float getExhaustVelocity() const;
    glm::vec3 getThrustDirection() const;

    // Setter
    void setThrustDirection(const glm::vec3&);
    void setPosition(const glm::vec3& pos) { position = pos; }
    void setVelocity(const glm::vec3& vel) { velocity = vel; }

    // Prediction
    void predictTrajectory(float, float);
};

#endif