#include "core/body.h"

// Use as State
Body::Body() : config_(Config()), name(""), mass(0.0f), position(0.0f), velocity(0.0f) {
};

Body::Body(const Config& config, std::shared_ptr<ILogger> logger)
    : config_(config), logger_(logger) {
    if (!logger_) {
        throw std::invalid_argument("Logger cannot be null");
    }
};

Body::Body(const Config& config, std::shared_ptr<ILogger> logger, const std::string& name, float mass, const glm::vec3& position, const glm::vec3& velocity)
    : config_(config), logger_(logger), name(name), mass(mass), position(position), velocity(velocity) {
    if (!logger_) {
        throw std::invalid_argument("Logger cannot be null");
    }
}

Body::Body(const Body& other) 
    : config_(other.config_), name(other.name), mass(other.mass), position(other.position), velocity(other.velocity) {
}

void Body::update(float deltaTime) {
    trajectory_->update(position, deltaTime);
}

void Body::render(const Shader& shader) const {
    // TODO: Ensure position is in the correct coordinate system
    if(trajectory_) {
        trajectory_->render(shader);
    }
}

Body& Body::operator=(const Body& other) {
    if (this != &other) {
        position = other.position;
        velocity = other.velocity;
        mass = other.mass;
    }
    return *this;
}

void Body::setTrajectory(std::unique_ptr<Trajectory> trajectory) {
    trajectory_ = std::move(trajectory);
}