#include "simulation.h"

#include <iostream>

Simulation::Simulation() : config(Config()), rocket(config), cameraDistance(20000.0f), timeScale(1.0f) {
}

Simulation::Simulation(Config& config) : rocket(config), cameraDistance(20000.0f), timeScale(1.0f) {
    R_e = config.physics_earth_radius;
}

Simulation::~Simulation() = default;

void Simulation::init() {
    rocket.init();

    // Generate Earth's sphere
    
    const int stacks = 20;        // Latitude segments
    const int slices = 20;        // Longitude segments
    std::vector<GLfloat> earthVertices;
    std::vector<GLuint> earthIndices;

    for (int i = 0; i <= stacks; ++i) {
        float theta = i * M_PI / stacks;
        for (int j = 0; j <= slices; ++j) {
            float phi = j * 2.0f * M_PI / slices;
            float x = R_e * std::sin(theta) * std::cos(phi);
            float y = R_e * std::cos(theta);
            float z = R_e * std::sin(theta) * std::sin(phi);
            earthVertices.push_back(x);
            earthVertices.push_back(y);
            earthVertices.push_back(z);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;
            earthIndices.push_back(first);
            earthIndices.push_back(second);
            earthIndices.push_back(first + 1);
            earthIndices.push_back(second);
            earthIndices.push_back(second + 1);
            earthIndices.push_back(first + 1);
        }
    }

    earth = std::make_unique<RenderObject>(earthVertices, earthIndices);
    std::cout << "Earth initialized: " << (earth ? "valid" : "null") << std::endl;
}

void Simulation::update(float deltaTime) {
    rocket.update(deltaTime * timeScale);
}

void Simulation::render(const Shader& shader) const {
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    float sceneHeight = height * 0.8f;

    // Scale factor (physical unit meters to rendering unit kilometers)
    const float scale = 0.001f; // 1 meter = 0.001 rendering units (i.e., 1 km = 1 unit)

    glm::vec3 cameraPos(0.0f, cameraDistance, cameraDistance);
    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / sceneHeight, 0.1f, cameraDistance * 2.0f);

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    // Render the Earth (scaled)
    glm::mat4 earthModel = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
    shader.setMat4("model", earthModel);
    shader.setVec4("color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    if (earth) {
        earth->render();
    } else {
        std::cerr << "Earth is null!" << std::endl;
    }

    // Render the rocket (scaled)
    rocket.render(shader);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error in Simulation::render: " << err << std::endl;
    }
}

void Simulation::setTimeScale(float ts) { 
    timeScale = std::max(ts, 0.1f); 
}

void Simulation::adjustTimeScale(float delta) { 
    timeScale = std::max(0.1f, std::min(timeScale + delta, 100.0f)); 
}

void Simulation::adjustCameraDistance(float delta) { 
    cameraDistance = std::max(std::min(cameraDistance + delta, 30000.0f), 12500.0f); 
}

float Simulation::getTimeScale() const { 
    return timeScale; 
}

Rocket& Simulation::getRocket() { 
    return rocket; 
}