#ifndef ROCKET_H
#define ROCKET_H

#define GLM_ENABLE_EXPERIMENTAL

#include "body.h"
#include "app/config.h"
#include "core/flight_plan.h"
#include "core/octree.h"
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
    float predictionDuration = 0.0f, predictionStep = 0.0f; // Prediction parameters
    float predictionTimer_ = 0.0f;            // Timer for prediction update frequency
    float predictionUpdateInterval_ = 2.0f;   // Update prediction every 2 seconds (reduced frequency)

    // Cache the initial state used for the last prediction.
    // If the rocket's state hasn't changed significantly since the last prediction,
    // we skip the expensive recalculation (500 RK4 steps × 4 force evaluations each).
    bool predictionDirty_ = true;             // Force first prediction
    glm::dvec3 lastPredPos_ = glm::dvec3(0.0);
    glm::dvec3 lastPredVel_ = glm::dvec3(0.0);
    double lastPredThrust_ = 0.0;
    double lastPredFuelMass_ = 0.0;

    // Check if prediction needs to be recalculated based on state changes
    bool needsPredictionUpdate() const;

    double fuel_mass;           // Fuel mass (kg)
    double thrust;              // Thrust (N)
    double exhaust_velocity;    // Exhaust velocity (m/s)
    float time = 0.0f;                // Time (s)
    glm::dvec3 thrustDirection = glm::dvec3(0.0); // Thrust direction (in local frame relative to Earth surface)
    bool launched = false;             // Whether the rocket is launched
    bool crashed_ = false;             // Whether the rocket has crashed
    glm::dvec3 earthPosition_;  // Earth position for altitude calculations in heliocentric coordinates

    // Convert a local thrust direction (relative to Earth surface at rocket position)
    // to world-space direction in heliocentric coordinates.
    // Local frame: Y = radially outward from Earth center, X/Z = tangential.
    glm::dvec3 localToWorldDirection(const glm::dvec3& localDir) const;
    
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
    glm::dvec3 computeAccelerationRK4(double currentMass, const BODY_MAP& bodies, const Octree* octree = nullptr) const;
    glm::dvec3 computeAccelerationAt(const glm::dvec3& pos, const glm::dvec3& vel, double currentMass, const BODY_MAP& bodies, const Octree* octree = nullptr) const;
    void updateTrajectory();
    glm::vec3 offsetPosition() const;
    glm::vec3 offsetPosition(const glm::dvec3&) const;
    Body updateStateRK4(const Body& state, double deltaTime, double& currentMass, double& currentFuel, const BODY_MAP& bodies, const Octree* octree = nullptr) const;

public:
    Rocket(const Config&, std::shared_ptr<ILogger> logger, const FlightPlan&);

    // Public functions
    void init();
    void update(float, const BODY_MAP&, const Octree* octree = nullptr);
    void render(const Shader&) const;
    void render(const Shader&, const glm::dvec3& renderOrigin) const;
    void toggleLaunch();
    void resetTime();

    // Getters
    glm::dvec3 getPosition() const;
    glm::dvec3 getVelocity() const;
    glm::vec3 getRenderPosition() const;  // Get position in rendering coordinates
    float getTime() const;
    bool isLaunched() const;
    bool isCrashed() const;
    double getMass() const;
    double getFuelMass() const;
    double getThrust() const;
    double getExhaustVelocity() const;
    glm::dvec3 getThrustDirection() const;

    // Setter
    void setThrustDirection(const glm::dvec3&);
    void setPosition(const glm::dvec3& pos) { position = pos; }
    void setVelocity(const glm::dvec3& vel) { velocity = vel; }
    void setEarthPosition(const glm::dvec3& pos) { earthPosition_ = pos; }

    // Prediction
    void predictTrajectory(float, float, const BODY_MAP&, const Octree* octree = nullptr);
};

#endif