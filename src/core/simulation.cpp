#include "core/simulation.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

Simulation::Simulation() : config(Config()), rocket(config, FlightPlan(config.flight_plan_path)), 
    cameraDistance(20000.0f), cameraPitch(45.0f), cameraYaw(45.0f), timeScale(1.0f),
    moonPos(0.0f, 384400000.0f, 0.0f),
    /*
     * NOTE: The logger is initialized with a default SpdlogLogger instance
     * which can be replaced with a custom logger if needed.
     */
    logger_(std::make_shared<SpdlogLogger>()) {
}

Simulation::Simulation(Config& config, std::shared_ptr<ILogger> logger) 
    : config(config), rocket(config, FlightPlan(config.flight_plan_path)), 
      cameraDistance(500000.0f), cameraPitch(45.0f), cameraYaw(45.0f), timeScale(1.0f),
      moonPos(0.0f, 384400000.0f, 0.0f),
      logger_(logger) {
    logger_->set_level((LogLevel)config.logger_level);
    if (!logger_) {
        LOG_ERROR(logger_, "Simulation", "Logger is null!");
        throw std::runtime_error("Logger is null");
    }
}

Simulation::~Simulation() = default;

void Simulation::init() {
    LOG_DEBUG(logger_, "Simulation", "Initializing simulation...");
    
    bodies["earth"] = std::make_unique<Body>(glm::vec3(0.0f), glm::vec3(0.0f), config.physics_earth_mass);
    // Moon
    bodies["moon"] = std::make_unique<Body>(moonPos, glm::vec3(-1022.0f, 0.0f, 0.0f), config.physics_moon_mass);
    if (!bodies["earth"] || !bodies["moon"]) {
        LOG_ERROR(logger_, "Simulation", "Failed to initialize earth or moon!");
        return;
    }

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
            float x = config.physics_earth_radius * std::sin(theta) * std::cos(phi);
            float y = config.physics_earth_radius * std::cos(theta);
            float z = config.physics_earth_radius * std::sin(theta) * std::sin(phi);
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
    if (earthVertices.empty() || earthIndices.empty()) {
        LOG_ERROR(logger_, "Simulation", "Empty vertices or indices for earth!");
        return;
    }
    
    try {
        bodies["earth"]->renderObject = std::make_unique<RenderObject>(earthVertices, earthIndices);
        LOG_INFO(logger_, "Simulation", "Earth renderObject created: vertices=" + 
                 std::to_string(earthVertices.size()) + ", indices=" + std::to_string(earthIndices.size()));
    } catch (const std::exception& e) {
        LOG_ERROR(logger_, "Simulation", "Error creating earth renderObject: " + std::string(e.what()));
        return;
    }

    // moon
    std::vector<GLfloat> moonVertices;
    for (size_t i = 0; i < earthVertices.size(); i += 3) {
        moonVertices.push_back(earthVertices[i] * (config.physics_moon_radius / config.physics_earth_radius));
        moonVertices.push_back(earthVertices[i + 1] * (config.physics_moon_radius / config.physics_earth_radius));
        moonVertices.push_back(earthVertices[i + 2] * (config.physics_moon_radius / config.physics_earth_radius));
    }
    if (moonVertices.empty()) {
        LOG_ERROR(logger_, "Simulation", "Empty moonVertices!");
        return;
    }
    
    try {
        bodies["moon"]->renderObject = std::make_unique<RenderObject>(moonVertices, earthIndices);
        LOG_INFO(logger_, "Simulation", "Moon renderObject created: vertices=" + 
                 std::to_string(moonVertices.size()) + ", indices=" + std::to_string(earthIndices.size()));
    } catch (const std::exception& e) {
        LOG_ERROR(logger_, "Simulation", "Error creating moon renderObject: " + std::string(e.what()));
        return;
    }
    LOG_INFO(logger_, "Simulation", "Earth initialized: " + std::string(bodies.find("earth") != bodies.end() ? "valid" : "null"));
    LOG_INFO(logger_, "Simulation", "Moon initialized: " + std::string(bodies.find("moon") != bodies.end() ? "valid" : "null"));
    LOG_INFO(logger_, "Simulation", "Map objects initialized");
}

void Simulation::update(float deltaTime) {
    static float elapsed_time = 0.0f;
    elapsed_time += deltaTime * timeScale;
    float dt = deltaTime * timeScale;
    
    // Step 1: Update all celestial bodies (Velocity Verlet)
    std::vector<std::pair<std::string, glm::vec3>> current_accs;
    for (const auto& [name, body] : bodies) {
        current_accs.emplace_back(name, computeBodyAcceleration(*body, bodies));
    }
    
    // Update positions
    for (const auto& [name, body] : bodies) {
        body->position += body->velocity * dt + 0.5f * current_accs[std::distance(bodies.begin(), bodies.find(name))].second * dt * dt;
    }
    
    // Calculate new accelerations and update velocities
    for (auto& [name, body] : bodies) {
        glm::vec3 new_acc = computeBodyAcceleration(*body, bodies);
        body->velocity += 0.5f * (current_accs[std::distance(bodies.begin(), bodies.find(name))].second + new_acc) * dt;
        
        // Check for NaN
        if (std::isnan(body->position.x) || std::isnan(body->velocity.x)) {
            LOG_ERROR(logger_, "Simulation", "NaN detected in " + name + ": Pos=" + 
                      glm::to_string(body->position) + ", Vel=" + glm::to_string(body->velocity));
        }
    }
    
    rocket.update(dt, bodies);
    
    float moon_radius = glm::length(bodies["moon"]->position);
    LOG_ORBIT(logger_, "Moon", elapsed_time, bodies["moon"]->position, moon_radius, bodies["moon"]->velocity);
    LOG_DEBUG(logger_, "Simulation", "Rocket: Pos=" + glm::to_string(rocket.getPosition()));
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
    glm::vec3 target = glm::vec3(0.0f, 384400000.0f * scale / 2, 0.0f); // middle of the Earth and Moon
    float radPitch = glm::radians(cameraPitch);
    float radYaw = glm::radians(cameraYaw);
    glm::vec3 cameraPos;
    cameraPos.x = target.x + cameraDistance * cos(radPitch) * sin(radYaw);
    cameraPos.y = target.y + cameraDistance * sin(radPitch);
    cameraPos.z = target.z + cameraDistance * cos(radPitch) * cos(radYaw);

    glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / sceneHeight, 0.1f, cameraDistance * 2.0f);

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    // Render the Earth (scaled)
    glm::mat4 earthModel = glm::translate(glm::mat4(1.0f), bodies.at("earth")->position * scale);
    earthModel = glm::scale(earthModel, glm::vec3(scale, scale, scale));
    shader.setMat4("model", earthModel);
    shader.setVec4("color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    if (bodies.find("earth") != bodies.end() && bodies.at("earth")->renderObject) {
        bodies.at("earth")->renderObject->render();
    } else {
        LOG_ERROR(logger_, "Simulation", "Earth is null or has no renderObject!");
    }

    rocket.render(shader);

    
    if (bodies.find("moon") != bodies.end() && bodies.at("moon")->renderObject) {
        glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), bodies.at("moon")->position * scale);
        moonModel = glm::scale(moonModel, glm::vec3(scale, scale, scale));
        shader.setMat4("model", moonModel);
        shader.setVec4("color", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
        bodies.at("moon")->renderObject->render();
    } else {
        LOG_ERROR(logger_, "Simulation", "Earth is null or has no renderObject!");
    }

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOG_ERROR(logger_, "Simulation", "OpenGL error in render: " + std::to_string(err));
    }
}

void Simulation::setTimeScale(float ts) { 
    timeScale = std::max(ts, 0.1f); 
    LOG_INFO(logger_, "Simulation", "Time scale set to " + std::to_string(ts));
}

void Simulation::adjustTimeScale(float delta) { 
    timeScale = std::max(0.1f, std::min(timeScale + delta, 100.0f)); 
    LOG_INFO(logger_, "Simulation", "Time scale adjusted to " + std::to_string(timeScale));
}

void Simulation::adjustCameraDistance(float delta) { 
    cameraDistance = std::max(std::min(cameraDistance + delta, 500000.0f), 12500.0f); 
    LOG_INFO(logger_, "Simulation", "Camera distance adjusted to " + std::to_string(cameraDistance));
}

void Simulation::adjustCameraRotation(float deltaPitch, float deltaYaw) {
    cameraPitch += deltaPitch;
    cameraYaw += deltaYaw;
    cameraPitch = glm::clamp(cameraPitch, -89.0f, 89.0f); // Limit the pitch angle to avoid flipping
    LOG_INFO(logger_, "Simulation", "Camera rotation: pitch=" + std::to_string(cameraPitch) + 
             ", yaw=" + std::to_string(cameraYaw));
}

glm::vec3 Simulation::computeBodyAcceleration(const Body& body, const BODY_MAP& bodies) const {
    glm::vec3 acc(0.0f);
    for (const auto& [name, other] : bodies) {
        if (other.get() != &body) {
            glm::vec3 delta = body.position - other->position;
            float r = glm::length(delta);
            if (r > 1e-6f) {
                acc -= (config.physics_gravity_constant * other->mass / (r * r * r)) * delta;
            } else {
                LOG_WARN(logger_, "Simulation", "Close encounter detected for " + name);
            }
        }
    }
    return acc;
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

const BODY_MAP& Simulation::getBodies() const {
    return bodies;
}