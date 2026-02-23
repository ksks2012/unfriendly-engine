#ifndef BODY_RENDERER_H
#define BODY_RENDERER_H

#include "rendering/render_object.h"
#include "rendering/shader.h"
#include "rendering/trajectory.h"
#include "logging/logger.h"

#include <glm/glm.hpp>
#include <memory>
#include <string>

/**
 * BodyRenderer - Handles all rendering aspects of a celestial body.
 * 
 * Separates rendering concerns (sphere mesh, orbit trajectory, color)
 * from the physics state (position, velocity, mass) held by Body.
 */
class BodyRenderer {
public:
    BodyRenderer() = default;
    ~BodyRenderer() = default;

    // Non-copyable (owns GPU resources), movable
    BodyRenderer(const BodyRenderer&) = delete;
    BodyRenderer& operator=(const BodyRenderer&) = delete;
    BodyRenderer(BodyRenderer&&) = default;
    BodyRenderer& operator=(BodyRenderer&&) = default;

    // --- Sphere mesh ---
    void setSphereRenderObject(std::unique_ptr<IRenderObject> obj) { sphereObject_ = std::move(obj); }
    bool hasSphere() const { return sphereObject_ != nullptr; }
    void renderSphere() const { if (sphereObject_) sphereObject_->render(); }

    // --- Orbit trajectory ---
    void setTrajectory(std::unique_ptr<Trajectory> trajectory) { trajectory_ = std::move(trajectory); }
    bool hasTrajectory() const { return trajectory_ != nullptr; }

    // Update trajectory with current position (called each physics step)
    void updateTrajectory(const glm::dvec3& position, double renderScale, float deltaTime) {
        if (trajectory_) {
            glm::vec3 scaledPos = glm::vec3(position * renderScale);
            trajectory_->update(scaledPos, deltaTime);
        }
    }

    // Render orbit trajectory
    void renderTrajectory(const Shader& shader) const {
        if (trajectory_) trajectory_->render(shader);
    }

    // Render orbit trajectory with center offset (e.g., Moon orbit centered on Earth)
    void renderTrajectory(const Shader& shader, const glm::vec3& orbitCenter) const {
        if (trajectory_) trajectory_->render(shader, orbitCenter);
    }

    // --- Prediction trajectory ---
    void setPrediction(std::unique_ptr<Trajectory> prediction) { prediction_ = std::move(prediction); }
    bool hasPrediction() const { return prediction_ != nullptr; }

    // For testing: expose internals
    void setRenderObject(std::unique_ptr<IRenderObject> obj) { sphereObject_ = std::move(obj); }

private:
    std::unique_ptr<IRenderObject> sphereObject_;   // Sphere mesh (planet/star geometry)
    std::unique_ptr<Trajectory> trajectory_;         // Orbit path visualization
    std::unique_ptr<Trajectory> prediction_;         // Predicted trajectory
};

#endif // BODY_RENDERER_H
