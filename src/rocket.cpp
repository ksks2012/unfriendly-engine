#include "rocket.h"
#include <vector>

Rocket::Rocket() 
    : mass(1000.0f),
      fuel_mass(500.0f),
      thrust(15000.0f),
      exhaust_velocity(3000.0f),
      position(0.0f, 0.0f, 0.0f),
      velocity(0.0f, 0.0f, 0.0f),
      time(0.0f),
      launched(false) {
}

void Rocket::init() {
    std::vector<GLfloat> vertices = {
        0.0f, 0.0f, 0.0f,
        0.0f, 100.0f, 0.0f,
        20.0f, 0.0f, 0.0f
    };
    std::vector<GLuint> indices = {0, 1, 2};
    renderObject = std::make_unique<RenderObject>(vertices, indices);
}

glm::vec3 Rocket::computeAcceleration(const State& state, float currentMass) const {
    const float gravity = 9.8f;
    
    // Thrust acceleration (Y-axis only)
    glm::vec3 thrust_acc(0.0f);
    if (fuel_mass > 0.0f && currentMass > 0.0f) {
        thrust_acc.y = thrust / currentMass;
    }

    glm::vec3 gravity_acc(0.0f, -gravity, 0.0f);

    // Air drag
    const float rho_0 = 1.225f, H = 8000.0f, Cd = 0.3f, A = 1.0f;
    float rho = rho_0 * std::exp(-state.position.y / H);
    float v_magnitude = glm::length(state.velocity);
    glm::vec3 drag_acc(0.0f);
    if (v_magnitude > 0.0f) {
        glm::vec3 v_unit = state.velocity / v_magnitude;
        float drag_force = 0.5f * rho * Cd * A * v_magnitude * v_magnitude;
        drag_acc = -drag_force * v_unit / currentMass;
    }

    // Wind force (assume horizontal wind speed of 5 m/s)
    const glm::vec3 wind_velocity(0.0f, 0.0f, 0.0f);
    glm::vec3 relative_velocity = state.velocity - wind_velocity;
    float rv_magnitude = glm::length(relative_velocity);
    glm::vec3 wind_drag_acc(0.0f);
    if (rv_magnitude > 0.0f) {
        glm::vec3 rv_unit = relative_velocity / rv_magnitude;
        float wind_drag_force = 0.5f * rho * Cd * A * rv_magnitude * rv_magnitude;
        wind_drag_acc = -wind_drag_force * rv_unit / currentMass;
    }

    return thrust_acc + gravity_acc + drag_acc + wind_drag_acc;
}

void Rocket::update(float deltaTime) {
    if (!launched) return;

    time += deltaTime;
    float fuel_consumption_rate = thrust / exhaust_velocity;
    float delta_fuel = fuel_consumption_rate * deltaTime;

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

    // Ground collision detection
    if (position.y <= 0.0f && velocity.y < 0.0f) {
        position.y = 0.0f;
        velocity = glm::vec3(0.0f);
        launched = false;
    }
}

void Rocket::render(const Shader& shader) const {
    shader.setMat4("model", glm::translate(glm::mat4(1.0f), position));
    shader.setVec4("color", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    if (renderObject) {
        renderObject->render();
    }
}

void Rocket::toggleLaunch() {
    launched = !launched;
    // NOTE: Wind force can be added here if needed
    if (!launched) {
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