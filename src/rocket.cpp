#include "rocket.h"

#include <vector>

Rocket::Rocket()
    : position(0.0f, 0.0f, 0.0f), 
      velocity(0.0f, 0.0f, 0.0f), 
      time(0.0f), 
      launched(false) {
}

void Rocket::init() {
    std::vector<GLfloat> vertices = {
        0.0f, 0.0f, 0.0f,
        0.0f, 100.0f, 0.0f,
        20.0f, 0.0f, 0.0f
    };
    std::vector<GLuint> indices = {0, 1, 2};
    renderObject = std::make_unique<RenderObject>(vertices, indices);
}

void Rocket::render(const Shader& shader) const {
    shader.setMat4("model", glm::translate(glm::mat4(1.0f), position));
    shader.setVec4("color", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    if (renderObject) {
        renderObject->render();
    }
}

void Rocket::toggleLaunch() {
    launched = !launched;
    if (!launched) {
        resetTime();
    }
}

void Rocket::resetTime() {
    time = 0.0f;
}

glm::vec3 Rocket::getPosition() const { 
    return position; 
}

glm::vec3 Rocket::getVelocity() const { 
    return velocity; 
}

float Rocket::getTime() const { 
    return time; 
}

bool Rocket::isLaunched() const { 
    return launched; 
}
