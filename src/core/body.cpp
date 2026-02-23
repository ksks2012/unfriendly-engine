#include "core/body.h"

// Use as State
Body::Body() : config_(Config()), name(""), mass(0.0), position(0.0), velocity(0.0) {
};

Body::Body(const Config& config, std::shared_ptr<ILogger> logger)
    : config_(config), logger_(logger) {
    if (!logger_) {
        throw std::invalid_argument("Logger cannot be null");
    }
};

Body::Body(const Config& config, std::shared_ptr<ILogger> logger, const std::string& name, double mass, const glm::dvec3& position, const glm::dvec3& velocity)
    : config_(config), logger_(logger), name(name), mass(mass), position(position), velocity(velocity) {
    if (!logger_) {
        throw std::invalid_argument("Logger cannot be null");
    }
}

Body::Body(const Body& other) 
    : config_(other.config_), name(other.name), mass(other.mass), position(other.position), velocity(other.velocity) {
}

void Body::update(float deltaTime) {
    renderer.updateTrajectory(position, static_cast<double>(config_.simulation_rendering_scale), deltaTime);
}

void Body::render(const Shader& shader) const {
    render(shader, glm::vec3(0.0f));
}

void Body::render(const Shader& shader, const glm::vec3& orbitCenter) const {
    renderer.renderTrajectory(shader, orbitCenter);
}

Body& Body::operator=(const Body& other) {
    if (this != &other) {
        position = other.position;
        velocity = other.velocity;
        mass = other.mass;
    }
    return *this;
}