#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include "app/config.h"
#include "rendering/render_object.h"
#include "rendering/shader.h"
#include "logging/logger.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Trajectory {
public:
    struct Config {
        size_t maxPoints;        // Maximum number of trajectory points
        float sampleInterval;    // Sampling interval (seconds)
        glm::vec4 color;         // Trajectory color
        float scale;             // Rendering scale factor
        float earthRadius;       // Earth's radius (meters)
    };

    Trajectory(const Config& config);
    Trajectory(const Config& config, std::shared_ptr<ILogger> logger);
    ~Trajectory() = default;

    void init();
    void update(const glm::vec3& position, float deltaTime);
    void render(const Shader& shader) const;
    void reset();
    void updateRenderObject();

    std::vector<glm::vec3> getPoints() const;
    float getSampleTimer() const;

    void setSampleTimer(float sampleTimer);
    void setPoints(std::vector<glm::vec3> points);

    // For testing
    void setRenderObject(std::unique_ptr<IRenderObject> renderObject) {
        renderObject_ = std::move(renderObject);
    }

private:
    Config config_;
    std::unique_ptr<IRenderObject> renderObject_;
    std::shared_ptr<ILogger> logger_;
    std::vector<glm::vec3> points_;
    size_t head_;
    size_t count_;
    float sampleTimer_;

    // Convert simulation coordinates to rendering coordinates
    glm::vec3 offsetPosition(const glm::vec3& position) const;
};

#endif