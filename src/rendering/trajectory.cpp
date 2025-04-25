#include "rendering/trajectory.h"
#include <GL/glew.h>

Trajectory::Trajectory(const Config& config)
    : config_(config), head_(0), count_(0), sampleTimer_(0.0f) {
    if (config_.maxPoints == 0) {
        throw std::invalid_argument("maxPoints cannot be zero");
    }
}

Trajectory::Trajectory(const Config& config, std::shared_ptr<ILogger> logger)
    : logger_(logger), config_(config), head_(0), count_(0), sampleTimer_(0.0f) {
    if (config_.maxPoints == 0) {
        throw std::invalid_argument("maxPoints cannot be zero");
    }
    if (!logger_) {
        throw std::invalid_argument("Logger cannot be null");
    }
}

void Trajectory::init() {
    std::vector<GLfloat> vertices(config_.maxPoints * 3, 0.0f);
    points_.resize(config_.maxPoints, glm::vec3(0.0f));
    if (!renderObject_) {
        try {
            renderObject_ = std::make_unique<RenderObject>(vertices, std::vector<GLuint>());
        } catch (const std::exception& e) {
            LOG_ERROR(logger_, "Trajectory", "Failed to create RenderObject: " + std::string(e.what()));
            throw;
        }
    }
}

void Trajectory::update(const glm::vec3& position, float deltaTime) {
    LOG_DEBUG(logger_, "Trajectory", "update");
    if (points_.empty()) {
        LOG_ERROR(logger_, "Trajectory", "points_ is empty");
        return;
    }
    if (std::isnan(deltaTime) || std::isinf(deltaTime)) {
        LOG_ERROR(logger_, "Trajectory", "Invalid deltaTime");
        return;
    }

    sampleTimer_ += deltaTime;
    if (sampleTimer_ == 0) {
        return;
    }
    sampleTimer_ = 0.0f;

    // Add new point
    points_[head_] = position;
    size_t offset = head_ * 3 * sizeof(GLfloat);
    GLfloat vertex[3] = {points_[head_].x, points_[head_].y, points_[head_].z};
    renderObject_->updateBuffer(offset, 3 * sizeof(GLfloat), vertex);

    head_ = (head_ + 1) % config_.maxPoints;
    if (count_ < config_.maxPoints) {
        count_++;
    }
    // TODO: Batch update
    // LOG_INFO(logger_, "Trajectory", "update end");
}

void Trajectory::render(const Shader& shader) const {
    if (count_ == 0) {
        return;
    }
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setVec4("color", config_.color);
    renderObject_->renderTrajectory(head_, count_, config_.maxPoints);
}

void Trajectory::reset() {
    head_ = 0;
    count_ = 0;
    sampleTimer_ = 0.0f;
    points_.assign(config_.maxPoints, glm::vec3(0.0f));
    std::vector<GLfloat> vertices(config_.maxPoints * 3, 0.0f);
    renderObject_->updateBuffer(0, vertices.size() * sizeof(GLfloat), vertices.data());
}

glm::vec3 Trajectory::offsetPosition(const glm::vec3& position) const {
    float altitude = glm::length(position) - config_.earthRadius;
    return glm::vec3(
        position.x * config_.scale,
        altitude * config_.scale + (config_.earthRadius * config_.scale),
        position.z * config_.scale
    );
}

void Trajectory::updateRenderObject() {
    std::vector<GLfloat> vertices;
    for (const auto& point : points_) {
        vertices.push_back(point.x);
        vertices.push_back(point.y);
        vertices.push_back(point.z);
    }
    renderObject_ = std::make_unique<RenderObject>(vertices, std::vector<GLuint>());
    if (!renderObject_) {
        LOG_ERROR(logger_, "Trajectory", "Failed to update RenderObject");
        throw std::runtime_error("Failed to update RenderObject");
    }
}

std::vector<glm::vec3> Trajectory::getPoints() const {
    return points_;
}

float Trajectory::getSampleTimer() const {
    return sampleTimer_;
}

void Trajectory::setSampleTimer(float sampleTimer) {
    sampleTimer_ = sampleTimer;
}