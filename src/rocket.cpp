#include "rocket.h"
#include <vector>
#include <cmath>

Rocket::Rocket() 
    : mass(1000.0f), fuel_mass(500.0f), thrust(15000.0f), exhaust_velocity(3000.0f),
      position(0.0f, 6371000.0f, 0.0f), // Initially on the surface (R_e = 6371 km)
      velocity(0.0f), time(0.0f), launched(false) {
}
void Rocket::init() {
    glm::vec3 renderPos = position * scale;

    std::vector<GLfloat> vertices = {
        0.0f, 0.0f, 0.0f,
        0.0f, 1000.0f, 0.0f,
        200.0f, 0.0f, 0.0f
    };
    std::vector<GLuint> indices = {0, 1, 2};
    renderObject = std::make_unique<RenderObject>(vertices, indices);

    // Initialize trajectory (empty data)
    trajectoryPoints.reserve(1000); // Reserve space
    trajectoryObject = std::make_unique<RenderObject>(std::vector<GLfloat>(), std::vector<GLuint>());
}

void Rocket::update(float deltaTime) {
    if (!launched) return;

    time += deltaTime;
    float fuel_consumption_rate = thrust / exhaust_velocity;
    float delta_fuel = fuel_consumption_rate * deltaTime;

    trajectorySampleTime += deltaTime;
    // Record trajectory every 0.1 seconds
    if (trajectorySampleTime >= 0.1f) {
        updateTrajectory();
        trajectorySampleTime = 0.0f;
    }

    // Initial state
    State current = {position, velocity};
    float current_mass = mass;

    // RK4 increments
    State k1, k2, k3, k4;
    k1.velocity = computeAcceleration(current, current_mass);
    k1.position = current.velocity;

    State mid1 = {current.position + k1.position * (deltaTime / 2.0f), 
                  current.velocity + k1.velocity * (deltaTime / 2.0f)};
    k2.velocity = computeAcceleration(mid1, current_mass);
    k2.position = mid1.velocity;

    State mid2 = {current.position + k2.position * (deltaTime / 2.0f), 
                  current.velocity + k2.velocity * (deltaTime / 2.0f)};
    k3.velocity = computeAcceleration(mid2, current_mass);
    k3.position = mid2.velocity;

    State end = {current.position + k3.position * deltaTime, 
                 current.velocity + k3.velocity * deltaTime};
    k4.velocity = computeAcceleration(end, current_mass);
    k4.position = end.velocity;

    // Update state
    position += (k1.position + 2.0f * k2.position + 2.0f * k3.position + k4.position) * (deltaTime / 6.0f);
    velocity += (k1.velocity + 2.0f * k2.velocity + 2.0f * k3.velocity + k4.velocity) * (deltaTime / 6.0f);

    // Update mass (after RK4 to simplify calculations)
    if (fuel_mass > 0.0f) {
        fuel_mass = std::max(0.0f, fuel_mass - delta_fuel);
        mass = mass - delta_fuel;
    }

    // Collision detection (prevent passing through Earth's surface)
    float r = glm::length(position);
    if (r < R_e) {
        position = glm::normalize(position) * R_e;
        velocity = glm::vec3(0.0f);
        launched = false;
    }
}

void Rocket::render(const Shader& shader) const {
    // Convert geocentric coordinates to local coordinates relative to the surface for rendering
    shader.setMat4("model", glm::translate(glm::mat4(1.0f), offsetPosition()));
    shader.setVec4("color", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    if (renderObject) 
        renderObject->render();

    // Render trajectory
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setVec4("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    if (trajectoryObject && !trajectoryPoints.empty()) {
        // Set before rendering
        glLineWidth(5.0f); 
        glBindVertexArray(trajectoryObject->getVao());
        glDrawArrays(GL_LINE_STRIP, 0, trajectoryPoints.size());
        // Restore default
        glLineWidth(1.0f); 
        glBindVertexArray(0);
    } else {
        if (!trajectoryObject) std::cerr << "trajectoryObject is null!" << std::endl;
    }
}

void Rocket::toggleLaunch() {
    launched = !launched;
    // NOTE: Wind force can be added here if needed
    // NOTE: Initial horizontal velocity to enter orbit (approximately the first cosmic velocity)
    if (launched) {
        trajectoryPoints.clear();
        trajectorySampleTime = 0.0f; // Reset the timer
    } else {
        resetTime();
        velocity = glm::vec3(0.0f); // Reset velocity
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

const std::vector<glm::vec3>& Rocket::getTrajectoryPoints() const { 
    return trajectoryPoints; 
}

// private

glm::vec3 Rocket::computeAcceleration(const State& state, float currentMass) const {
    // Gravitational force
    float r = glm::length(state.position);
    glm::vec3 gravity_acc = - (G * M / (r * r * r)) * state.position;

    // Thrust (assumed along the rocket's current direction, simplified as vertically upward)
    glm::vec3 thrust_acc(0.0f);
    if (fuel_mass > 0.0f && currentMass > 0.0f) {
        glm::vec3 thrust_dir = glm::normalize(state.position); // Pointing radially outward
        thrust_acc = (thrust / currentMass) * thrust_dir;
    }

    // Air resistance (significant below 100 km altitude)
    const float rho_0 = 1.225f, H = 8000.0f, Cd = 0.3f, A = 1.0f;
    float altitude = r - R_e;
    glm::vec3 drag_acc(0.0f);
    // Significant atmosphere below 100 km
    if (altitude < 100000.0f) { 
        float rho = rho_0 * std::exp(-altitude / H);
        float v_magnitude = glm::length(state.velocity);
        if (v_magnitude > 0.0f) {
            glm::vec3 v_unit = state.velocity / v_magnitude;
            float drag_force = 0.5f * rho * Cd * A * v_magnitude * v_magnitude;
            drag_acc = -drag_force * v_unit / currentMass;
        }
    }

    return gravity_acc + thrust_acc + drag_acc;
}

void Rocket::updateTrajectory() {
    if (trajectoryPoints.size() >= 1000) {
        trajectoryPoints.erase(trajectoryPoints.begin());
    }
    trajectoryPoints.push_back(offsetPosition());

    // Update trajectory buffer
    std::vector<GLfloat> vertices;
    for (const auto& point : trajectoryPoints) {
        vertices.push_back(point.x);
        vertices.push_back(point.y); // Offset for rendering
        vertices.push_back(point.z);
    }
    trajectoryObject = std::make_unique<RenderObject>(vertices, std::vector<GLuint>());
}

glm::vec3 Rocket::offsetPosition() const {
    // Offset position for rendering
    float altitude = glm::length(position) - R_e;
    return glm::vec3(position.x, altitude + (R_e * scale), position.z);
}