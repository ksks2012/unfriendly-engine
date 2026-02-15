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
    enum class RenderMode {
        LineStrip,  // For rocket trajectory (open path)
        LineLoop    // For orbital paths (closed loop)
    };

    struct Config {
        size_t maxPoints;        // Maximum number of trajectory points
        float sampleInterval;    // Sampling interval (seconds)
        glm::vec4 color;         // Trajectory color
        float scale;             // Rendering scale factor
        float earthRadius;       // Earth's radius (meters)
        RenderMode renderMode = RenderMode::LineStrip; // Render mode
        bool isStatic = false;   // If true, orbit is pre-calculated and won't be updated dynamically
    };

    Trajectory(const Config& config);
    Trajectory(const Config& config, std::shared_ptr<ILogger> logger);
    ~Trajectory() = default;

    void init();
    void update(const glm::vec3& position, float deltaTime);
    void render(const Shader& shader) const;
    void render(const Shader& shader, const glm::vec3& center) const;  // Render with center offset
    void reset();
    void updateRenderObject();
    
    void setCenter(const glm::vec3& center) { center_ = center; }  // Set orbit center

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
    glm::vec3 center_ = glm::vec3(0.0f);  // Orbit center point

    // Dirty region tracking for batched VBO upload.
    // Instead of calling glBufferSubData on every point insertion,
    // we record which indices were modified and flush them all at once
    // before rendering, reducing GPU upload calls from N to 1-2 per frame.
    bool dirty_ = false;
    size_t dirtyStart_ = 0;  // First modified index in the ring buffer
    size_t dirtyEnd_ = 0;    // One past the last modified index
    bool dirtyWrapped_ = false;  // True if dirty region wraps around the ring buffer

    // Flush pending point data to the GPU (called before render)
    void flushToGPU();

    // Mark an index as modified
    void markDirty(size_t index);

    // Convert simulation coordinates to rendering coordinates
    glm::vec3 offsetPosition(const glm::vec3& position) const;
};

#endif