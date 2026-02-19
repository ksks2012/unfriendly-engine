#ifndef BODY_H
#define BODY_H

#include "app/config.h"
#include "rendering/trajectory.h"

#include <glm/glm.hpp>
#include <string>


class Body {
protected:
    std::unique_ptr<Trajectory> trajectory_;
    std::unique_ptr<Trajectory> prediction_;

    const Config& config_;
    std::shared_ptr<ILogger> logger_;

public:
    std::string name;
    double mass;
    glm::dvec3 position;
    glm::dvec3 velocity;
    std::unique_ptr<IRenderObject> renderObject;

public:
    Body();
    Body(const Config& config, std::shared_ptr<ILogger> logger);
    Body(const Config& config, std::shared_ptr<ILogger> logger, const std::string& name, double mass, const glm::dvec3& position, const glm::dvec3& velocity);
    Body(const Body& other);
    virtual ~Body() = default;

    Body& operator=(const Body& other);

    virtual void update(float deltaTime);
    virtual void render(const Shader& shader) const;
    virtual void render(const Shader& shader, const glm::vec3& orbitCenter) const;  // Render orbit with center offset

    // Getters
    std::string getName() const { return name; }
    double getMass() const { return mass; }
    glm::dvec3 getPosition() const { return position; }
    glm::dvec3 getVelocity() const { return velocity; }

    // Setters
    void setPosition(const glm::dvec3& position) { this->position = position; }
    void setVelocity(const glm::dvec3& velocity) { this->velocity = velocity; }
    void setTrajectory(std::unique_ptr<Trajectory> trajectory);
};

typedef std::unordered_map<std::string, std::unique_ptr<Body>> BODY_MAP;

#endif