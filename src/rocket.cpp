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

void Rocket::update(float deltaTime) {
    if (!launched) 
        return;

    time += deltaTime;
    const float gravity = 9.8f;

    // Fuel consumption rate (kg/s) = thrust / exhaust velocity
    float fuel_consumption_rate = thrust / exhaust_velocity;
    float delta_fuel = fuel_consumption_rate * deltaTime;
    if (fuel_mass > 0.0f) {
        // Update fuel mass
        fuel_mass = std::max(0.0f, fuel_mass - delta_fuel);

        // Update total mass
        mass = mass - delta_fuel;
    }

    // Thrust acceleration (Y-axis only)
    glm::vec3 thrust_acc(0.0f);
    if (fuel_mass > 0.0f) {
        thrust_acc.y = thrust / mass;
    }

    // Gravitational acceleration
    glm::vec3 gravity_acc(0.0f, -gravity, 0.0f);

    // Air drag
    const float rho_0 = 1.225f;     // Air density at sea level (kg/m³)
    const float H = 8000.0f;        // Atmospheric scale height (m)
    const float Cd = 0.3f;          // Drag coefficient
    const float A = 1.0f;           // Cross-sectional area (m²)
    float rho = rho_0 * std::exp(-position.y / H); // Decreases with altitude
    float v_magnitude = glm::length(velocity);
    glm::vec3 drag_acc(0.0f);
    if (v_magnitude > 0.0f) {
        glm::vec3 v_unit = velocity / v_magnitude;
        float drag_force = 0.5f * rho * Cd * A * v_magnitude * v_magnitude;
        drag_acc = -drag_force * v_unit / mass;
    }

    // Wind force (assume horizontal wind speed of 5 m/s)
    const glm::vec3 wind_velocity(0.0f, 0.0f, 0.0f);
    glm::vec3 relative_velocity = velocity - wind_velocity;
    float rv_magnitude = glm::length(relative_velocity);
    glm::vec3 wind_drag_acc(0.0f);
    if (rv_magnitude > 0.0f) {
        glm::vec3 rv_unit = relative_velocity / rv_magnitude;
        float wind_drag_force = 0.5f * rho * Cd * A * rv_magnitude * rv_magnitude;
        wind_drag_acc = -wind_drag_force * rv_unit / mass;
    }

    // Total acceleration
    glm::vec3 acceleration = thrust_acc + gravity_acc + drag_acc + wind_drag_acc;

    // Update velocity and position
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;

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