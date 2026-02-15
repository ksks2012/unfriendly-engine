#include "rendering/trajectory.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

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
    // Static orbits (pre-calculated) should not be updated dynamically
    if (config_.isStatic) {
        return;
    }
    
    if (points_.empty()) {
        LOG_ERROR(logger_, "Trajectory", "points_ is empty");
        return;
    }
    if (std::isnan(deltaTime) || std::isinf(deltaTime)) {
        LOG_ERROR(logger_, "Trajectory", "Invalid deltaTime");
        return;
    }

    sampleTimer_ += deltaTime;
    if (sampleTimer_ < config_.sampleInterval) {
        return; // Not time to sample yet
    }
    
    // Reset timer but keep the remainder for accurate timing
    sampleTimer_ -= config_.sampleInterval;
    
    // Skip update if position hasn't changed significantly (optimization)
    if (count_ > 0) {
        size_t lastIdx = (head_ == 0) ? config_.maxPoints - 1 : head_ - 1;
        glm::vec3 lastPos = points_[lastIdx];
        float dist = glm::length(position - lastPos);
        // Skip if moved less than 1 meter (in rendering scale)
        if (dist < 0.001f) {
            return;
        }
    }

    // Add new point (CPU side only, defer GPU upload to flushToGPU)
    points_[head_] = position;
    markDirty(head_);

    head_ = (head_ + 1) % config_.maxPoints;
    if (count_ < config_.maxPoints) {
        count_++;
    }
}

void Trajectory::markDirty(size_t index) {
    if (!dirty_) {
        // First dirty point this frame
        dirty_ = true;
        dirtyStart_ = index;
        dirtyEnd_ = index + 1;
        dirtyWrapped_ = false;
    } else {
        // Extend dirty region
        size_t nextEnd = index + 1;
        if (nextEnd <= dirtyStart_) {
            // New point is before the current start — wrap-around detected
            dirtyWrapped_ = true;
            dirtyEnd_ = nextEnd;
        } else {
            dirtyEnd_ = nextEnd;
        }
    }
}

void Trajectory::flushToGPU() {
    if (!dirty_ || !renderObject_) {
        return;
    }

    if (!dirtyWrapped_) {
        // Contiguous region: single upload
        size_t startOffset = dirtyStart_ * 3 * sizeof(GLfloat);
        size_t count = dirtyEnd_ - dirtyStart_;
        std::vector<GLfloat> buffer(count * 3);
        for (size_t i = 0; i < count; ++i) {
            const auto& p = points_[dirtyStart_ + i];
            buffer[i * 3 + 0] = p.x;
            buffer[i * 3 + 1] = p.y;
            buffer[i * 3 + 2] = p.z;
        }
        renderObject_->updateBuffer(
            static_cast<GLintptr>(startOffset),
            static_cast<GLsizei>(buffer.size() * sizeof(GLfloat)),
            buffer.data());
    } else {
        // Wrapped region: two uploads (tail portion + head portion)
        // Part 1: from dirtyStart_ to end of buffer
        size_t tailCount = config_.maxPoints - dirtyStart_;
        if (tailCount > 0) {
            size_t startOffset = dirtyStart_ * 3 * sizeof(GLfloat);
            std::vector<GLfloat> buffer(tailCount * 3);
            for (size_t i = 0; i < tailCount; ++i) {
                const auto& p = points_[dirtyStart_ + i];
                buffer[i * 3 + 0] = p.x;
                buffer[i * 3 + 1] = p.y;
                buffer[i * 3 + 2] = p.z;
            }
            renderObject_->updateBuffer(
                static_cast<GLintptr>(startOffset),
                static_cast<GLsizei>(buffer.size() * sizeof(GLfloat)),
                buffer.data());
        }
        // Part 2: from 0 to dirtyEnd_
        if (dirtyEnd_ > 0) {
            std::vector<GLfloat> buffer(dirtyEnd_ * 3);
            for (size_t i = 0; i < dirtyEnd_; ++i) {
                const auto& p = points_[i];
                buffer[i * 3 + 0] = p.x;
                buffer[i * 3 + 1] = p.y;
                buffer[i * 3 + 2] = p.z;
            }
            renderObject_->updateBuffer(
                0,
                static_cast<GLsizei>(buffer.size() * sizeof(GLfloat)),
                buffer.data());
        }
    }

    // Reset dirty state
    dirty_ = false;
    dirtyStart_ = 0;
    dirtyEnd_ = 0;
    dirtyWrapped_ = false;
}

void Trajectory::render(const Shader& shader) const {
    render(shader, center_);
}

void Trajectory::render(const Shader& shader, const glm::vec3& center) const {
    LOG_DEBUG(logger_, "Trajectory", "render");
    if (count_ == 0) {
        return;
    }
    
    // Flush any pending point data to GPU before drawing
    // (const_cast is safe here: flushToGPU only mutates internal dirty tracking
    // state and GPU buffer, not the logical trajectory data)
    const_cast<Trajectory*>(this)->flushToGPU();
    
    // Apply translation to orbit center
    glm::mat4 model = glm::translate(glm::mat4(1.0f), center);
    shader.setMat4("model", model);
    shader.setVec4("color", config_.color);
    
    if (config_.renderMode == RenderMode::LineLoop) {
        // Use LINE_LOOP for closed orbits
        renderObject_->renderOrbit(count_);
    } else {
        // Use LINE_STRIP for open trajectories
        renderObject_->renderTrajectory(head_, count_, config_.maxPoints);
    }
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

void Trajectory::setPoints(std::vector<glm::vec3> points) {
    if (points.size() > config_.maxPoints) {
        points.resize(config_.maxPoints);
    }
    points_ = std::move(points);
    head_ = points_.size() - 1.0f;
    count_ = points_.size();
    updateRenderObject();
    LOG_INFO(logger_, "Trajectory", "setPoints: head_=" + std::to_string(head_) + ", count_=" + std::to_string(count_));
}