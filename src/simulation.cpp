#include "simulation.h"
#include <iostream>
#include <vector>

Simulation::Simulation() : config(Config()), rocket(config, FlightPlan(config.flight_plan_path)), 
    cameraDistance(20000.0f), cameraPitch(45.0f), cameraYaw(45.0f), timeScale(1.0f) {
}

Simulation::Simulation(Config& config) : config(config), rocket(config, FlightPlan(config.flight_plan_path)), 
    cameraDistance(500000.0f), cameraPitch(45.0f), cameraYaw(45.0f), timeScale(1.0f),
    moonPos(0.0f, 384400000.0f, 0.0f), moonAngularSpeed(2.665e-6f), moonMass(7.342e22f) {
    R_e = config.physics_earth_radius;
    // TODO: to config
    R_moon = 1737400.0f;

    // Earth
    bodies["Earth"] = Body(glm::vec3(0.0f), glm::vec3(0.0f), config.physics_earth_mass);
    // Moon
    bodies["Moon"] = Body(moonPos, glm::vec3(-1022.0f, 0.0f, 0.0f), moonMass);
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
    bodies["Earth"].renderObject = std::make_unique<RenderObject>(earthVertices, earthIndices);

    // moon
    float R_moon = 1737400.0f;
    std::vector<GLfloat> moonVertices;
    for (size_t i = 0; i < earthVertices.size(); i += 3) {
        moonVertices.push_back(earthVertices[i] * (R_moon / R_e));
        moonVertices.push_back(earthVertices[i + 1] * (R_moon / R_e));
        moonVertices.push_back(earthVertices[i + 2] * (R_moon / R_e));
    }
    bodies["Moon"].renderObject = std::make_unique<RenderObject>(moonVertices, earthIndices);

    std::cout << "Earth initialized: " << (bodies.find("Earth") != bodies.end() ? "valid" : "null") << std::endl;
    std::cout << "Moon initialized: " << (bodies.find("Moon")  != bodies.end() ? "valid" : "null") << std::endl;
    std::cout << "Map objects initialized" << std::endl;
}

void Simulation::update(float deltaTime) {
    rocket.update(deltaTime * timeScale);

    // Update the moon's position
    float angle = moonAngularSpeed * deltaTime * timeScale;
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
    moonPos = glm::vec3(rotation * glm::vec4(moonPos, 1.0f));
}

void Simulation::updateCameraPosition() const {
    // TODO: Follow the rocket
}

void Simulation::render(const Shader& shader) const {
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    float sceneHeight = height * 0.8f;

    // Scale factor (physical unit meters to rendering unit kilometers)
    const float scale = 0.001f; // 1 meter = 0.001 rendering units (i.e., 1 km = 1 unit)
    // glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f); // Earth's center
    glm::vec3 target = glm::vec3(0.0, 384400000.0f * scale / 2, 0.0f); // middle of the Earth and Moon
    float radPitch = glm::radians(cameraPitch);
    float radYaw = glm::radians(cameraYaw);
    glm::vec3 cameraPos;
    cameraPos.x = target.x + cameraDistance * cos(radPitch) * sin(radYaw);
    cameraPos.y = target.y + cameraDistance * sin(radPitch);
    cameraPos.z = target.z + cameraDistance * cos(radPitch) * cos(radYaw);

    // Generate view and projection matrices
    glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / sceneHeight, 0.1f, cameraDistance * 2.0f);

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    // Render the Earth (scaled)
    glm::mat4 earthModel = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
    shader.setMat4("model", earthModel);
    shader.setVec4("color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    if (bodies.find("Earth") != bodies.end()) {
        bodies.at("Earth").renderObject->render();
    } else {
        std::cerr << "Earth is null!" << std::endl;
    }

    // Render the rocket (scaled)
    rocket.render(shader);

    glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), moonPos * scale);
    moonModel = glm::scale(moonModel, glm::vec3(scale, scale, scale));
    shader.setMat4("model", moonModel);
    shader.setVec4("color", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
    if (bodies.find("Moon") != bodies.end()) {
        bodies.at("Moon").renderObject->render();
    } else {
        std::cerr << "Moon is null!" << std::endl;
    }

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
    cameraDistance = std::max(std::min(cameraDistance + delta, 500000.0f), 12500.0f); 
}

void Simulation::adjustCameraRotation(float deltaPitch, float deltaYaw) {
    cameraPitch += deltaPitch;
    cameraYaw += deltaYaw;
    cameraPitch = glm::clamp(cameraPitch, -89.0f, 89.0f); // Limit the pitch angle to avoid flipping
}

float Simulation::getTimeScale() const { 
    return timeScale; 
}

Rocket& Simulation::getRocket() { 
    return rocket; 
}

glm::vec3 Simulation::getMoonPos() const {
    return moonPos;
}

const std::unordered_map<std::string, Body>& Simulation::getBodies() const {
    return bodies;
}
