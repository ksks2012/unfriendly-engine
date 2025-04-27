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
    float mass;
    glm::vec3 position;
    glm::vec3 velocity;
    std::unique_ptr<IRenderObject> renderObject;

public:
    Body();
    Body(const Config& config, std::shared_ptr<ILogger> logger);
    Body(const Config& config, std::shared_ptr<ILogger> logger, const std::string& name, float mass, const glm::vec3& position, const glm::vec3& velocity);
    Body(const Body& other);
    virtual ~Body() = default;

    Body& operator=(const Body& other);

    virtual void update(float deltaTime);
    virtual void render(const Shader& shader) const;

    // Getters
    std::string getName() const { return name; }
    float getMass() const { return mass; }
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getVelocity() const { return velocity; }

    // Setters
    void setPosition(const glm::vec3& position) { this->position = position; }
    void setVelocity(const glm::vec3& velocity) { this->velocity = velocity; }
    void setTrajectory(std::unique_ptr<Trajectory> trajectory);
};

typedef std::unordered_map<std::string, std::unique_ptr<Body>> BODY_MAP;

#endif