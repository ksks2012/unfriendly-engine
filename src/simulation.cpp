#include "simulation.h"

#include <iostream>

Simulation::Simulation() : rocketPos(0.0f, 0.0f, 0.0f), cameraDistance(200.0f), timeScale(1.0f) {
}

void Simulation::init() {
    std::vector<GLfloat> rocketVertices = {
        0.0f, 0.0f, 0.0f,
        0.0f, 100.0f, 0.0f,
        20.0f, 0.0f, 0.0f
    };
    std::vector<GLuint> rocketIndices = {0, 1, 2};
    rocket = std::make_unique<RenderObject>(rocketVertices, rocketIndices);

    std::vector<GLfloat> groundVertices = {
        -500.0f, 0.0f, -500.0f,
        500.0f, 0.0f, -500.0f,
        500.0f, 0.0f, 500.0f,
        -500.0f, 0.0f, 500.0f
    };
    std::vector<GLuint> groundIndices = {0, 1, 2, 0, 2, 3};
    ground = std::make_unique<RenderObject>(groundVertices, groundIndices);
}

void Simulation::update() {
    // TODO: Future physics updates go here
}

void Simulation::render(const Shader& shader) const {
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    float sceneHeight = height * 0.8f;

    glm::vec3 cameraPos(0.0f, cameraDistance, cameraDistance);
    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / sceneHeight, 0.1f, cameraDistance * 2.0f);

    shader.use();

    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    // Render ground
    shader.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
    shader.setVec4("color", glm::vec4(0.0f, 0.8f, 0.0f, 1.0f));
    ground->render();

    // Render rocket
    shader.setMat4("model", glm::translate(glm::mat4(1.0f), rocketPos));
    shader.setVec4("color", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    rocket->render();

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
    }
}

void Simulation::setTimeScale(float ts) { 
    timeScale = std::max(ts, 0.1f); 
}

void Simulation::adjustTimeScale(float delta) { 
    timeScale = std::max(0.1f, std::min(timeScale + delta, 100.0f)); 
}

void Simulation::adjustCameraDistance(float delta) { 
    cameraDistance = std::max(std::min(cameraDistance + delta, 3000.0f), 50.0f); 
}

float Simulation::getTimeScale() const { 
    return timeScale; 
}
