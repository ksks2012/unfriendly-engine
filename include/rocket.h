#ifndef ROCKET_H
#define ROCKET_H

#include "shader.h"
#include "render_object.h"

#include <glm/glm.hpp>
#include <memory>

class Rocket {
public:
    Rocket();

    void init();
    
    void render(const Shader& shader) const;
    void toggleLaunch();
    void resetTime();

    // Getter
    glm::vec3 getPosition() const;
    glm::vec3 getVelocity() const;
    float getTime() const;
    bool isLaunched() const;

private:
    std::unique_ptr<RenderObject> renderObject;
    glm::vec3 position;
    glm::vec3 velocity;
    float time;
    bool launched;
};

#endif