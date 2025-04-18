#include "rocket.h"
#include <vector>
#include <cmath>

#include <iostream>

Rocket::Rocket(const Config& config, const FlightPlan& plan) 
    : config(config), flightPlan(plan),
      mass(config.rocket_mass), fuel_mass(config.rocket_fuel_mass),
      thrust(config.rocket_thrust), exhaust_velocity(config.rocket_exhaust_velocity),
      position(config.rocket_initial_position), velocity(config.rocket_initial_velocity),
      time(0.0f), launched(false), trajectorySampleTime(0.0f) {
}

void Rocket::init() {
    thrustDirection = glm::vec3(0.0f, 1.0f, 0.0f); // Thrust direction (upward)

    std::vector<GLfloat> vertices = {
        0.0f, 0.0f, 0.0f,
        0.0f, 1000.0f, 0.0f,
        200.0f, 0.0f, 0.0f
    };
    std::vector<GLuint> indices = {0, 1, 2};
    if(!renderObject)
        renderObject = std::make_unique<RenderObject>(vertices, indices);

    std::vector<GLfloat> trajectoryVertices(TRAJECTORY_SIZE * 3, 0.0f);
    if(!trajectoryObject)
        trajectoryObject = std::make_unique<RenderObject>(trajectoryVertices, std::vector<GLuint>());
    trajectoryPoints.fill(glm::vec3(0.0f));

    if(!predictionObject)
        predictionObject = std::make_unique<RenderObject>(std::vector<GLfloat>(), std::vector<GLuint>());
    predictionPoints.reserve(PREDICTION_SIZE);
}

void Rocket::update(float deltaTime, const BODY_MAP& bodies) {
    if (!launched) return;
    time += deltaTime;
    trajectorySampleTime += deltaTime;
    if (trajectorySampleTime >= 0.1f) {
        updateTrajectory();
        predictTrajectory(config.simulation_prediction_duration, config.simulation_prediction_step);
        trajectorySampleTime = 0.0f;
    }
    
    Body current = *this;
    float currentMass = mass;
    Body newState = updateStateRK4(current, deltaTime, currentMass, fuel_mass, bodies);
    position = newState.position;
    velocity = newState.velocity;
    mass = currentMass;
    
    float r = glm::length(position);
    float altitude = r - config.physics_earth_radius;
    if (altitude < 0.0f) {
        position = glm::normalize(position) * config.physics_earth_radius;
        velocity = glm::vec3(0.0f);
        launched = false;
    }
    
    auto action = flightPlan.getAction(altitude, glm::length(velocity));
    if (action) {
        thrust = action->thrust;
        thrustDirection = action->direction;
    }
}

void Rocket::render(const Shader& shader) const {
    // Render the rocket
    shader.setMat4("model", glm::translate(glm::mat4(1.0f), offsetPosition()));
    shader.setVec4("color", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    if (renderObject) 
        renderObject->render();

    // Render trajectory (delegated to RenderObject)
    shader.setMat4("model", glm::mat4(1.0f)); // set model matrix to identity
    shader.setVec4("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // set color to red
    if (trajectoryObject && trajectoryCount > 0) {
        trajectoryObject->renderTrajectory(trajectoryHead, trajectoryCount, TRAJECTORY_SIZE);
    }

    // Render prediction trajectory (delegated to RenderObject)
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setVec4("color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // set color to green
    if (predictionObject && !predictionPoints.empty()) {
        predictionObject->renderTrajectory(0, PREDICTION_SIZE - 1, PREDICTION_SIZE);
    }
}

void Rocket::toggleLaunch() {
    launched = !launched;
    // NOTE: Wind force can be added here if needed
    // NOTE: Initial horizontal velocity to enter orbit (approximately the first cosmic velocity)
    if (launched) {
        trajectoryHead = 0;
        trajectoryCount = 0;
        trajectorySampleTime = 0.0f; // Reset the timer
    } else {
        resetTime();
    }
}

void Rocket::resetTime() {
    time = 0.0f;
}

glm::vec3 Rocket::getPosition() const { 
    return position; 
}

glm::vec3 Rocket::getVelocity() const { 
    return velocity; 
}

float Rocket::getTime() const { 
    return time; 
}

bool Rocket::isLaunched() const { 
    return launched; 
}

float Rocket::getMass() const { 
    return mass; 
}

float Rocket::getFuelMass() const { 
    return fuel_mass; 
}

float Rocket::getThrust() const { 
    return thrust; 
}

float Rocket::getExhaustVelocity() const { 
    return exhaust_velocity; 
}

glm::vec3 Rocket::getThrustDirection() const {
    return thrustDirection;
}

void Rocket::setThrustDirection(const glm::vec3& direction) {
    thrustDirection = glm::normalize(direction);
    if (glm::length(velocity) > 0.0f) {
        velocity = glm::normalize(thrustDirection) * glm::length(velocity);
    }
}

void Rocket::setRenderObjects(std::unique_ptr<IRenderObject> render,
    std::unique_ptr<IRenderObject> trajectory,
    std::unique_ptr<IRenderObject> prediction) {
    renderObject = std::move(render);
    trajectoryObject = std::move(trajectory);
    predictionObject = std::move(prediction);
}

// private

glm::vec3 Rocket::computeAccelerationRK4(float currentMass, const BODY_MAP& bodies) const {
    glm::vec3 acc(0.0f);
    for (const auto& [name, body] : bodies) {
        if (body.get() != this) {
            glm::vec3 delta = position - body->position;
            float r = glm::length(delta);
            if (r > 1e-6f) {
                acc -= (config.physics_gravity_constant * body->mass / (r * r * r)) * delta;
            }
        }
    }
    
    if (fuel_mass > 0.0f && currentMass > 0.0f) {
        acc += (thrust / currentMass) * thrustDirection;
    }
    
    float r = glm::length(position);
    float altitude = r - config.physics_earth_radius;
    if (altitude < 100000.0f) {
        float rho = config.physics_air_density * std::exp(-altitude / config.physics_scale_height);
        float v_magnitude = glm::length(velocity);
        if (v_magnitude > 0.0f) {
            glm::vec3 v_unit = velocity / v_magnitude;
            float drag_force = 0.5f * rho * config.physics_drag_coefficient * config.physics_cross_section_area * v_magnitude * v_magnitude;
            acc -= drag_force * v_unit / currentMass;
        }
    }
    
    std::cout << "Rocket: Acc=" << glm::to_string(acc) << ", Fuel=" << fuel_mass << std::endl;
    return acc;
}

void Rocket::updateTrajectory() {
    trajectoryPoints[trajectoryHead] = offsetPosition();
    size_t offset = trajectoryHead * 3 * sizeof(GLfloat);
    size_t bufferSize = TRAJECTORY_SIZE * 3 * sizeof(GLfloat);

    if (offset >= bufferSize) {
        std::cerr << "Error: Offset exceeds buffer size!" << std::endl;
        return;
    }

    // Update VBO
    GLfloat vertex[3] = {trajectoryPoints[trajectoryHead].x, 
                         trajectoryPoints[trajectoryHead].y, 
                         trajectoryPoints[trajectoryHead].z};
    trajectoryObject->updateBuffer(offset, 3 * sizeof(GLfloat), vertex);

    trajectoryHead = (trajectoryHead + 1) % TRAJECTORY_SIZE;
    if (trajectoryCount < TRAJECTORY_SIZE) {
        trajectoryCount++;
    }

}

glm::vec3 Rocket::offsetPosition() const {
    // Offset position for rendering
    float altitude = glm::length(position) - config.physics_earth_radius;
    return glm::vec3(position.x * config.simulation_rendering_scale, altitude * config.simulation_rendering_scale + (config.physics_earth_radius * config.simulation_rendering_scale), position.z * config.simulation_rendering_scale);
}

glm::vec3 Rocket::offsetPosition(glm::vec3 inputPosition) const {
    // Offset position for rendering
    float altitude = glm::length(inputPosition) - config.physics_earth_radius;
    return glm::vec3(inputPosition.x * config.simulation_rendering_scale, altitude * config.simulation_rendering_scale + (config.physics_earth_radius * config.simulation_rendering_scale), inputPosition.z * config.simulation_rendering_scale);
}

void Rocket::predictTrajectory(float duration, float step) {
    predictionPoints.clear();
    Body state = *this;
    float predMass = mass;
    float predFuel = fuel_mass;
    float predTime = 0.0f;
    
    while (predTime < duration && predFuel >= 0.0f) {
        predictionPoints.push_back(offsetPosition(state.position));
        state = updateStateRK4(state, step, predMass, predFuel, {});
        predTime += step;
    }
    
    std::vector<GLfloat> vertices;
    for (const auto& point : predictionPoints) {
        vertices.push_back(point.x);
        vertices.push_back(point.y);
        vertices.push_back(point.z);
    }
    predictionObject = std::make_unique<RenderObject>(vertices, std::vector<GLuint>());
}

Body Rocket::updateStateRK4(const Body& state, float deltaTime, float& currentMass, float& currentFuel, const BODY_MAP& bodies) const {
    float fuel_consumption_rate = thrust / exhaust_velocity;
    float delta_fuel = fuel_consumption_rate * deltaTime;
    
    Body k1, k2, k3, k4;
    k1.velocity = computeAccelerationRK4(currentMass, bodies);
    k1.position = state.velocity;
    
    Body mid1;
    mid1.position = state.position + k1.position * (deltaTime / 2.0f);
    mid1.velocity = state.velocity + k1.velocity * (deltaTime / 2.0f);
    k2.velocity = computeAccelerationRK4(currentMass, bodies);
    k2.position = mid1.velocity;
    
    Body mid2;
    mid2.position = state.position + k2.position * (deltaTime / 2.0f);
    mid2.velocity = state.velocity + k2.velocity * (deltaTime / 2.0f);
    k3.velocity = computeAccelerationRK4(currentMass, bodies);
    k3.position = mid2.velocity;
    
    Body end;
    end.position = state.position + k3.position * deltaTime;
    end.velocity = state.velocity + k3.velocity * deltaTime;
    k4.velocity = computeAccelerationRK4(currentMass, bodies);
    k4.position = end.velocity;
    
    Body newState;
    newState.position = state.position + (k1.position + 2.0f * k2.position + 2.0f * k3.position + k4.position) * (deltaTime / 6.0f);
    newState.velocity = state.velocity + (k1.velocity + 2.0f * k2.velocity + 2.0f * k3.velocity + k4.velocity) * (deltaTime / 6.0f);
    
    if (currentFuel > 0.0f) {
        currentFuel = std::max(0.0f, currentFuel - delta_fuel);
        currentMass = currentMass - delta_fuel;
    }
    
    return newState;
}