#include "core/rocket.h"
#include "logging/spdlog_logger.h"
#include <vector>
#include <cmath>

#include <iostream>

Rocket::Rocket(const Config& config, std::shared_ptr<ILogger> logger, const FlightPlan& plan)
     : flightPlan(plan), fuel_mass(config.rocket_fuel_mass),
        thrust(config.rocket_thrust), exhaust_velocity(config.rocket_exhaust_velocity),
        earthPosition_(0.0),  // Will be updated from simulation
        Body(config, logger, "Rocket", config.rocket_mass, config.rocket_initial_position, config.rocket_initial_velocity) {
    if (!logger_) {
        throw std::runtime_error("[Rocket] Logger is null");
    }
}

void Rocket::init() {
    thrustDirection = glm::dvec3(0.0, 1.0, 0.0); // Thrust direction (upward)

    // Rocket 3D pyramid vertices (in rendering units: km)
    // Made larger so it's visible at typical camera distances (100-500 km)
    // Pyramid with tip pointing in +Y direction (forward)
    // Center the pyramid so rotation works correctly around origin
    const float baseSize = 10.0f;  // Base width/depth: 20 km (±10 km)
    const float height = 50.0f;    // Height: 50 km
    const float tipY = height * 0.75f;   // Tip at 3/4 height above center
    const float baseY = -height * 0.25f; // Base at 1/4 height below center
    
    std::vector<GLfloat> vertices = {
        // Tip of the pyramid (forward direction)
        0.0f, tipY, 0.0f,                 // 0: Tip
        
        // Base vertices (square)
        -baseSize, baseY, -baseSize,      // 1: Back-left
        baseSize, baseY, -baseSize,       // 2: Back-right
        baseSize, baseY, baseSize,        // 3: Front-right
        -baseSize, baseY, baseSize        // 4: Front-left
    };
    
    // Indices for 4 triangular faces + 2 triangles for base
    std::vector<GLuint> indices = {
        // Side faces (4 triangles from tip to base edges)
        0, 4, 3,  // Front face
        0, 3, 2,  // Right face
        0, 2, 1,  // Back face
        0, 1, 4,  // Left face
        
        // Base (2 triangles to close the bottom)
        1, 2, 3,
        1, 3, 4
    };
    
    if(!renderObject)
        renderObject = std::make_unique<RenderObject>(vertices, indices);

    if(!trajectory_)
        trajectory_ = TrajectoryFactory::createRocketTrajectory(config_, logger_);
    trajectory_->init();

    if(!prediction_)
        prediction_ = TrajectoryFactory::createRocketPredictionTrajectory(config_, logger_);
    prediction_->init();
}

void Rocket::update(float deltaTime, const BODY_MAP& bodies, const Octree* octree) {
    // Update Earth position for altitude calculations
    glm::dvec3 previousEarthPosition = earthPosition_;
    if (bodies.find("earth") != bodies.end()) {
        earthPosition_ = bodies.at("earth")->position;
    }
    
    if (!launched) {
        // When not launched, rocket should follow Earth's movement
        // Calculate how much Earth has moved and apply the same delta to rocket
        glm::dvec3 earthDelta = earthPosition_ - previousEarthPosition;
        position += earthDelta;
        
        // Also update velocity to match Earth's orbital velocity
        if (bodies.find("earth") != bodies.end()) {
            velocity = bodies.at("earth")->velocity;
        }
        
        // Still update prediction trajectory even when not launched
        // This allows user to see predicted path before launch
        if (prediction_ && deltaTime > 0.0f) {
            predictionTimer_ += deltaTime;
            if (predictionTimer_ >= predictionUpdateInterval_) {
                predictTrajectory(config_.simulation_prediction_duration, config_.simulation_prediction_step, bodies, octree);
                predictionTimer_ = 0.0f;
            }
        }
        return;
    }
    
    time += deltaTime;
    
    if (trajectory_ && deltaTime > 0.0f) {
        LOG_DEBUG(logger_, "Rocket", "Trajectory trajectory_ update");
        trajectory_->update(offsetPosition(position), deltaTime);
        
        // Update prediction less frequently to improve performance
        predictionTimer_ += deltaTime;
        if (predictionTimer_ >= predictionUpdateInterval_) {
            predictTrajectory(config_.simulation_prediction_duration, config_.simulation_prediction_step, bodies, octree);
            predictionTimer_ = 0.0f;
        }
    }
    
    Body current = *this;
    double currentMass = mass;
    Body newState = updateStateRK4(current, static_cast<double>(deltaTime), currentMass, fuel_mass, bodies, octree);
    position = newState.position;
    velocity = newState.velocity;
    mass = currentMass;
    
    // Calculate altitude relative to Earth (not Sun) in heliocentric coordinates
    glm::dvec3 relativeToEarth = position - earthPosition_;
    double distanceFromEarthCenter = glm::length(relativeToEarth);
    double altitude = distanceFromEarthCenter - config_.physics_earth_radius;
    if (altitude < 0.0) {
        // Crashed into Earth: clamp position to surface, match Earth velocity
        LOG_INFO(logger_, "Rocket", "Crashed into Earth at altitude " + std::to_string(altitude));
        glm::dvec3 dirFromEarth = glm::normalize(relativeToEarth);
        position = earthPosition_ + dirFromEarth * config_.physics_earth_radius;
        // Match Earth orbital velocity so the rocket stays on the surface
        if (bodies.find("earth") != bodies.end()) {
            velocity = bodies.at("earth")->velocity;
        } else {
            velocity = glm::dvec3(0.0);
        }
        launched = false;
        crashed_ = true;
        predictionDirty_ = true;
    }
    
    auto action = flightPlan.getAction(altitude, glm::length(velocity));
    if (action) {
        thrust = action->thrust;
        thrustDirection = action->direction;
        predictionDirty_ = true;  // Thrust/direction change invalidates prediction
    }
}

void Rocket::render(const Shader& shader) const {
    render(shader, glm::dvec3(0.0));
}

void Rocket::render(const Shader& shader, const glm::dvec3& renderOrigin) const {
    // Calculate rotation matrix to align rocket with velocity direction
    glm::mat4 model = glm::mat4(1.0f);
    
    // Calculate velocity direction in rendering coordinate system
    // Use numerical differentiation: direction = d(offsetPosition)/dt
    // which is approximately (offsetPosition(pos + vel*dt) - offsetPosition(pos)) / dt
    glm::vec3 direction;
    if (glm::length(velocity) > 0.1) {
        // Small time step for numerical derivative
        const double dt = 0.01;  // 10ms
        
        // Current render position (relative to renderOrigin)
        glm::vec3 currentRenderPos = glm::vec3((position - renderOrigin) * static_cast<double>(config_.simulation_rendering_scale));
        
        // Future position in physics coordinates
        glm::dvec3 futurePhysicsPos = position + velocity * dt;
        
        // Future render position (relative to renderOrigin)
        glm::vec3 futureRenderPos = glm::vec3((futurePhysicsPos - renderOrigin) * static_cast<double>(config_.simulation_rendering_scale));
        
        // Velocity direction in render coordinates
        glm::vec3 renderVelocity = futureRenderPos - currentRenderPos;
        
        if (glm::length(renderVelocity) > 0.0001f) {
            direction = glm::normalize(renderVelocity);
        } else {
            direction = glm::vec3(0.0f, 1.0f, 0.0f);  // Default upward
        }
    } else {
        // When stationary, point upward (in rendering Y direction)
        direction = glm::vec3(0.0f, 1.0f, 0.0f);
    }
    
    // Default forward direction is +Y (the pyramid tip points in +Y)
    glm::vec3 defaultForward = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // Calculate rotation from default forward to actual direction
    float dotProduct = glm::dot(defaultForward, direction);
    
    // Build rotation matrix first
    glm::mat4 rotation = glm::mat4(1.0f);
    if (dotProduct < -0.999f) {
        // Nearly opposite direction: rotate 180 degrees around any perpendicular axis
        glm::vec3 rotAxis = glm::vec3(1.0f, 0.0f, 0.0f);  // Use X axis
        rotation = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), rotAxis);
    } else if (dotProduct < 0.999f) {
        // General case: calculate rotation axis and angle
        glm::vec3 rotAxis = glm::cross(defaultForward, direction);
        float rotAxisLen = glm::length(rotAxis);
        if (rotAxisLen > 0.0001f) {
            rotAxis = glm::normalize(rotAxis);
            float angle = acos(glm::clamp(dotProduct, -1.0f, 1.0f));
            rotation = glm::rotate(glm::mat4(1.0f), angle, rotAxis);
        }
    }
    // If dotProduct >= 0.999f, no rotation needed (already aligned)
    
    // Apply transformations: first rotate (around origin), then translate
    // Position is computed relative to renderOrigin for float precision
    glm::vec3 relativePos = glm::vec3((position - renderOrigin) * static_cast<double>(config_.simulation_rendering_scale));
    model = glm::translate(model, relativePos);
    model = model * rotation;
    
    shader.setMat4("model", model);
    shader.setVec4("color", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    if (renderObject) 
        renderObject->render();

    // Render trajectory (delegated to RenderObject)
    trajectory_->render(shader);

    // Render prediction trajectory (delegated to RenderObject)
    prediction_->render(shader);
}

void Rocket::toggleLaunch() {
    launched = !launched;
    crashed_ = false;  // Clear crash state on any launch toggle
    predictionDirty_ = true;  // Force prediction recalculation on launch state change
    // NOTE: Wind force can be added here if needed
    // NOTE: Initial horizontal velocity to enter orbit (approximately the first cosmic velocity)
    if (!launched) {
        resetTime();
    }
}

void Rocket::resetTime() {
    time = 0.0f;
}

glm::dvec3 Rocket::getPosition() const { 
    return position; 
}

glm::dvec3 Rocket::getVelocity() const { 
    return velocity; 
}

glm::vec3 Rocket::getRenderPosition() const {
    // Return position in rendering coordinates (same as used for rendering)
    return offsetPosition();
}

float Rocket::getTime() const { 
    return time; 
}

bool Rocket::isLaunched() const { 
    return launched; 
}

bool Rocket::isCrashed() const {
    return crashed_;
}

double Rocket::getMass() const { 
    return mass; 
}

double Rocket::getFuelMass() const { 
    return fuel_mass; 
}

double Rocket::getThrust() const { 
    return thrust; 
}

double Rocket::getExhaustVelocity() const { 
    return exhaust_velocity; 
}

glm::dvec3 Rocket::getThrustDirection() const {
    return thrustDirection;
}

void Rocket::setThrustDirection(const glm::dvec3& direction) {
    thrustDirection = glm::normalize(direction);
    if (glm::length(velocity) > 0.0) {
        velocity = glm::normalize(thrustDirection) * glm::length(velocity);
    }
}

// For unit tests
void Rocket::setRender(std::unique_ptr<IRenderObject> render) {
    renderObject = std::move(render);
}

void Rocket::setTrajectoryRender(std::unique_ptr<IRenderObject> trajectory, std::unique_ptr<IRenderObject> prediction) {
    if (!trajectory_) {
        trajectory_ = TrajectoryFactory::createRocketTrajectory(config_, logger_);
    }
    trajectory_->setRenderObject(std::move(trajectory));

    if (!prediction_) {
        prediction_ = TrajectoryFactory::createRocketTrajectory(config_, logger_);
    }
    prediction_->setRenderObject(std::move(prediction));
}

// private

glm::dvec3 Rocket::computeAccelerationRK4(double currentMass, const BODY_MAP& bodies, const Octree* octree) const {
    // This version uses the rocket's current position/velocity (for real-time update)
    return computeAccelerationAt(position, velocity, currentMass, bodies, octree);
}

glm::dvec3 Rocket::computeAccelerationAt(const glm::dvec3& pos, const glm::dvec3& vel, double currentMass, const BODY_MAP& bodies, const Octree* octree) const {
    glm::dvec3 acc(0.0);
    
    // Gravity from all bodies: use Barnes-Hut octree if available, else direct summation
    if (octree) {
        acc = octree->computeAcceleration(pos, config_.physics_gravity_constant);
    } else {
        for (const auto& [name, body] : bodies) {
            if (body.get() != this) {
                glm::dvec3 delta = pos - body->position;
                double r = glm::length(delta);
                if (r > 1e-6) {
                    acc -= (config_.physics_gravity_constant * body->mass / (r * r * r)) * delta;
                }
            }
        }
    }
    
    // Thrust: convert local-frame thrustDirection to world-space
    if (fuel_mass > 0.0 && currentMass > 0.0) {
        glm::dvec3 worldThrust = localToWorldDirection(thrustDirection);
        acc += (thrust / currentMass) * worldThrust;
    }
    
    // Atmospheric drag (relative to Earth)
    glm::dvec3 relativeToEarth = pos - earthPosition_;
    double distFromEarthCenter = glm::length(relativeToEarth);
    double altitude = distFromEarthCenter - config_.physics_earth_radius;
    if (altitude > 0.0 && altitude < 100000.0) {
        double rho = config_.physics_air_density * std::exp(-altitude / config_.physics_scale_height);
        double v_magnitude = glm::length(vel);
        if (v_magnitude > 0.0) {
            glm::dvec3 v_unit = vel / v_magnitude;
            double drag_force = 0.5 * rho * config_.physics_drag_coefficient * config_.physics_cross_section_area * v_magnitude * v_magnitude;
            acc -= drag_force * v_unit / currentMass;
        }
    }
    
    LOG_DEBUG(logger_, "Rocket", "Acc=" + glm::to_string(glm::vec3(acc)) + ", Fuel=" + std::to_string(fuel_mass));
    return acc;
}

glm::vec3 Rocket::offsetPosition() const {
    // In heliocentric coordinate system, just scale the position
    // The position is already in meters (dvec3), convert to km (vec3) for rendering
    return glm::vec3(position * static_cast<double>(config_.simulation_rendering_scale));
}

glm::vec3 Rocket::offsetPosition(const glm::dvec3& inputPosition) const {
    // In heliocentric coordinate system, just scale the position
    return glm::vec3(inputPosition * static_cast<double>(config_.simulation_rendering_scale));
}

glm::dvec3 Rocket::localToWorldDirection(const glm::dvec3& localDir) const {
    // Build a local coordinate frame at the rocket's position relative to Earth.
    // Local Y = radially outward from Earth center (the "up" direction at launch site)
    // Local X = arbitrary tangential direction (eastward-like)
    // Local Z = completes the right-handed frame
    glm::dvec3 radial = position - earthPosition_;
    double r = glm::length(radial);
    if (r < 1e-6) {
        return localDir;  // Degenerate: rocket at Earth center
    }
    
    glm::dvec3 up = radial / r;  // Local Y: radially outward
    
    // Choose a reference vector that's not parallel to up for cross product
    glm::dvec3 ref = (std::abs(glm::dot(up, glm::dvec3(0, 0, 1))) < 0.99)
                   ? glm::dvec3(0, 0, 1) : glm::dvec3(1, 0, 0);
    
    glm::dvec3 east = glm::normalize(glm::cross(ref, up));  // Local X: tangential
    glm::dvec3 north = glm::cross(up, east);                 // Local Z: completes frame
    
    // Transform: world_dir = east * localDir.x + up * localDir.y + north * localDir.z
    return east * localDir.x + up * localDir.y + north * localDir.z;
}

bool Rocket::needsPredictionUpdate() const {
    // Always compute on first call
    if (predictionDirty_) return true;

    // Position change threshold: if the rocket has moved significantly
    // relative to its orbit, the old prediction is stale.
    // Use a fraction of Earth's radius as a meaningful distance.
    constexpr double posThreshold = 1000.0;       // 1 km
    constexpr double velThreshold = 1.0;           // 1 m/s
    constexpr double thrustEps    = 0.01;          // Thrust on/off change
    constexpr double fuelEps      = 0.01;          // Fuel consumption change

    double dPos = glm::length(position - lastPredPos_);
    double dVel = glm::length(velocity - lastPredVel_);
    double dThrust = std::abs(thrust - lastPredThrust_);
    double dFuel = std::abs(fuel_mass - lastPredFuelMass_);

    return dPos > posThreshold || dVel > velThreshold
        || dThrust > thrustEps || dFuel > fuelEps;
}

void Rocket::predictTrajectory(float duration, float step, const BODY_MAP& bodies, const Octree* octree) {
    LOG_DEBUG(logger_, "Rocket", "predictTrajectory");

    // Skip expensive recalculation if state hasn't changed significantly
    if (!needsPredictionUpdate()) {
        LOG_DEBUG(logger_, "Rocket", "Prediction skipped - state unchanged");
        return;
    }

    // Cache the state we're basing this prediction on
    lastPredPos_      = position;
    lastPredVel_      = velocity;
    lastPredThrust_   = thrust;
    lastPredFuelMass_ = fuel_mass;
    predictionDirty_  = false;

    // Reset prediction trajectory before recalculating
    prediction_->reset();
    
    Body state = *this;
    double predMass = mass;
    double predFuel = fuel_mass;
    float predTime = 0.0f;
    
    // Adaptive step control parameters
    const int maxPoints = 500;  // Limit total points for performance
    int pointCount = 0;
    
    // Skip factor: only add every Nth point to trajectory for rendering
    // This allows fine physics simulation while keeping render points low
    const float renderInterval = std::max(step, duration / maxPoints);
    float timeSinceLastRender = 0.0f;
    
    // Calculate prediction points
    while (predTime < duration && pointCount < maxPoints) {
        // Only add point at render intervals
        if (timeSinceLastRender >= renderInterval || predTime == 0.0f) {
            glm::vec3 scaledPos = offsetPosition(state.position);
            // Pass renderInterval as deltaTime to ensure point is added
            // (must be >= sampleInterval in Trajectory config)
            prediction_->update(scaledPos, renderInterval);
            pointCount++;
            timeSinceLastRender = 0.0f;
        }
        
        // Calculate altitude relative to Earth (not Sun) in heliocentric coordinates
        glm::dvec3 relativeToEarth = state.position - earthPosition_;
        double distanceFromEarthCenter = glm::length(relativeToEarth);
        double altitude = distanceFromEarthCenter - config_.physics_earth_radius;
        
        // Stop if crashed or escaped too far from Earth
        if (altitude < 0.0 || altitude > config_.physics_moon_distance * 2.0) {
            break;
        }
        
        // Adaptive step size based on altitude
        // Use larger steps at higher altitudes where dynamics change slowly
        float adaptiveStep = step;
        if (altitude > 100000.0) {
            // Above 100 km: can use larger steps
            adaptiveStep = step * 2.0f;
        }
        if (altitude > 1000000.0) {
            // Above 1000 km: use even larger steps
            adaptiveStep = step * 5.0f;
        }
        
        // Use actual bodies for gravity calculation in prediction
        state = updateStateRK4(state, adaptiveStep, predMass, predFuel, bodies, octree);
        predTime += adaptiveStep;
        timeSinceLastRender += adaptiveStep;
    }
}

Body Rocket::updateStateRK4(const Body& state, double deltaTime, double& currentMass, double& currentFuel, const BODY_MAP& bodies, const Octree* octree) const {
    double fuel_consumption_rate = thrust / exhaust_velocity;
    double delta_fuel = fuel_consumption_rate * deltaTime;
    
    // RK4 integration using correct intermediate positions and velocities
    Body k1, k2, k3, k4;
    
    // k1: acceleration at current state
    k1.velocity = computeAccelerationAt(state.position, state.velocity, currentMass, bodies, octree);
    k1.position = state.velocity;
    
    // k2: acceleration at midpoint using k1
    Body mid1;
    mid1.position = state.position + k1.position * (deltaTime / 2.0);
    mid1.velocity = state.velocity + k1.velocity * (deltaTime / 2.0);
    k2.velocity = computeAccelerationAt(mid1.position, mid1.velocity, currentMass, bodies, octree);
    k2.position = mid1.velocity;
    
    // k3: acceleration at midpoint using k2
    Body mid2;
    mid2.position = state.position + k2.position * (deltaTime / 2.0);
    mid2.velocity = state.velocity + k2.velocity * (deltaTime / 2.0);
    k3.velocity = computeAccelerationAt(mid2.position, mid2.velocity, currentMass, bodies, octree);
    k3.position = mid2.velocity;
    
    // k4: acceleration at endpoint using k3
    Body end;
    end.position = state.position + k3.position * deltaTime;
    end.velocity = state.velocity + k3.velocity * deltaTime;
    k4.velocity = computeAccelerationAt(end.position, end.velocity, currentMass, bodies, octree);
    k4.position = end.velocity;
    
    // Combine the four estimates
    Body newState;
    newState.position = state.position + (k1.position + 2.0 * k2.position + 2.0 * k3.position + k4.position) * (deltaTime / 6.0);
    newState.velocity = state.velocity + (k1.velocity + 2.0 * k2.velocity + 2.0 * k3.velocity + k4.velocity) * (deltaTime / 6.0);
    
    // Update fuel consumption
    if (currentFuel > 0.0) {
        currentFuel = std::max(0.0, currentFuel - delta_fuel);
        currentMass = currentMass - delta_fuel;
    }
    
    return newState;
}