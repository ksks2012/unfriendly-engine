#ifndef SIMULATION_H
#define SIMULATION_H

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

#include "render_object.h"
#include "shader.h"

class Simulation {
public:
    Simulation() : rocketPos(0.0f, 0.0f, 0.0f), cameraDistance(200.0f), timeScale(1.0f) {
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

    void update() {
        // TODO: Future physics updates go here
    }

    void render(const Shader& shader) const {
        int width, height;
        glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
        float sceneHeight = height * 0.8f;

        glm::vec3 cameraPos(0.0f, cameraDistance, cameraDistance);
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / sceneHeight, 0.1f, cameraDistance * 2.0f);

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
    }

    void setTimeScale(float ts) { timeScale = std::max(ts, 0.1f); }
    void adjustTimeScale(float delta) { timeScale = std::max(timeScale + delta, 0.1f); }
    void adjustCameraDistance(float delta) { 
        cameraDistance = std::max(std::min(cameraDistance + delta, 3000.0f), 50.0f); 
    }
    float getTimeScale() const { return timeScale; }

private:
    std::unique_ptr<RenderObject> rocket, ground;
    glm::vec3 rocketPos;
    float cameraDistance;
    float timeScale;
};

#endif