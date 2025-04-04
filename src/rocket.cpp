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

    // Accumulate time
    time += deltaTime;

    // Gravitational acceleration (m/s^2)
    const float gravity = 9.8f;

    // Fuel consumption rate (kg/s) = thrust / exhaust velocity
    float fuel_consumption_rate = thrust / exhaust_velocity;
    float delta_fuel = fuel_consumption_rate * deltaTime;

    if (fuel_mass > 0.0f) {
        // Update fuel mass
        fuel_mass = std::max(0.0f, fuel_mass - delta_fuel);

        // Update total mass
        mass = mass - delta_fuel;

        // Acceleration = (thrust - gravity force) / mass
        float acceleration = (thrust - mass * gravity) / mass;

        // Update velocity (only Y-axis upward)
        velocity.y += acceleration * deltaTime;

        // Update position
        position.y += velocity.y * deltaTime;
    } else {
        // Fuel is depleted, only affected by gravity
        velocity.y -= gravity * deltaTime;
        position.y += velocity.y * deltaTime;
    }

    // Stop motion if it returns to the ground
    if (position.y <= 0.0f && velocity.y < 0.0f) {
        position.y = 0.0f;
        velocity.y = 0.0f;
        launched = false; // Automatically stop launch
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