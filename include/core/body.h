#ifndef BODY_H
#define BODY_H

#include "app/config.h"
#include "rendering/body_renderer.h"

#include <glm/glm.hpp>
#include <string>


class Body {
public:
    // Physics state (public for direct access in physics integrator)
    std::string name;
    double mass;
    glm::dvec3 position;
    glm::dvec3 velocity;

    // Rendering component (owns sphere mesh + orbit trajectory)
    BodyRenderer renderer;

public:
    Body();
    Body(const Config& config, std::shared_ptr<ILogger> logger);
    Body(const Config& config, std::shared_ptr<ILogger> logger, const std::string& name, double mass, const glm::dvec3& position, const glm::dvec3& velocity);
    Body(const Body& other);
    virtual ~Body() = default;

    Body& operator=(const Body& other);

    // Update trajectory with current position (delegates to renderer)
    virtual void update(float deltaTime);

    // Render orbit trajectory (delegates to renderer)
    virtual void render(const Shader& shader) const;
    virtual void render(const Shader& shader, const glm::vec3& orbitCenter) const;

    // Getters
    std::string getName() const { return name; }
    double getMass() const { return mass; }
    glm::dvec3 getPosition() const { return position; }
    glm::dvec3 getVelocity() const { return velocity; }

    // Setters
    void setPosition(const glm::dvec3& position) { this->position = position; }
    void setVelocity(const glm::dvec3& velocity) { this->velocity = velocity; }

    // Convenience: delegate to renderer (backward compatibility)
    void setTrajectory(std::unique_ptr<Trajectory> trajectory) { renderer.setTrajectory(std::move(trajectory)); }

protected:
    const Config& config_;
    std::shared_ptr<ILogger> logger_;
};

typedef std::unordered_map<std::string, std::unique_ptr<Body>> BODY_MAP;

#endif