#include "core/simulation.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

/*
 * NOTE: The logger is initialized with a default SpdlogLogger instance
 * which can be replaced with a custom logger if needed.
 */
Simulation::Simulation(Camera &camera) 
    : config(Config()), logger_(std::make_shared<SpdlogLogger>()), rocket(config, logger_, FlightPlan(config.flight_plan_path)), 
        camera(camera), moonPos(0.0f, 384400000.0f, 0.0f) {
}

Simulation::Simulation(Config& config, std::shared_ptr<ILogger> logger, Camera& camera) : 
        config(config), rocket(config, logger, FlightPlan(config.flight_plan_path)), 
        logger_(logger), camera(camera), timeScale(1.0f), moonPos(0.0f, 384400000.0f, 0.0f) {
    if (!logger_) {
        throw std::runtime_error("Logger is null");
    }
    logger_->set_level((LogLevel)config.logger_level);
}

Simulation::~Simulation() = default;

void Simulation::init() {
    LOG_DEBUG(logger_, "Simulation", "Initializing simulation...");
    
    // Sun at origin (heliocentric coordinate system)
    bodies["sun"] = std::make_unique<Body>(config, logger_, "sun", config.physics_sun_mass, glm::vec3(0.0f), glm::vec3(0.0f));
    
    // Mercury - innermost planet
    // Orbital inclination: 7.005° to ecliptic
    {
        glm::vec3 pos = glm::vec3(config.physics_mercury_orbit_radius, 0.0f, 0.0f);
        glm::vec3 vel = glm::vec3(0.0f, 0.0f, config.physics_mercury_orbital_velocity);
        bodies["mercury"] = std::make_unique<Body>(config, logger_, "mercury", config.physics_mercury_mass, pos, vel);
        bodies["mercury"]->setTrajectory(TrajectoryFactory::createPlanetOrbit(
            config, logger_, config.physics_mercury_orbit_radius,
            glm::vec4(0.7f, 0.7f, 0.7f, 0.8f),  // Gray
            glm::radians(7.005f)
        ));
    }
    
    // Venus - second planet
    // Orbital inclination: 3.395° to ecliptic
    {
        glm::vec3 pos = glm::vec3(config.physics_venus_orbit_radius, 0.0f, 0.0f);
        glm::vec3 vel = glm::vec3(0.0f, 0.0f, config.physics_venus_orbital_velocity);
        bodies["venus"] = std::make_unique<Body>(config, logger_, "venus", config.physics_venus_mass, pos, vel);
        bodies["venus"]->setTrajectory(TrajectoryFactory::createPlanetOrbit(
            config, logger_, config.physics_venus_orbit_radius,
            glm::vec4(0.9f, 0.7f, 0.5f, 0.8f),  // Orange-ish
            glm::radians(3.395f)
        ));
    }
    
    // Earth orbiting the Sun (1 AU distance, orbital velocity ~29.78 km/s)
    glm::vec3 earthPos = glm::vec3(config.physics_earth_orbit_radius, 0.0f, 0.0f);
    glm::vec3 earthVel = glm::vec3(0.0f, 0.0f, config.physics_earth_orbital_velocity); // Perpendicular to position
    bodies["earth"] = std::make_unique<Body>(config, logger_, "earth", config.physics_earth_mass, earthPos, earthVel);
    
    // Moon orbiting Earth with ~5.145° orbital inclination relative to the ecliptic
    // The lunar orbital plane is tilted from the ecliptic plane
    const float lunarInclination = glm::radians(5.145f);
    
    // Moon position: in the inclined orbital plane
    // Start with position in the ecliptic (X-Z plane), then apply inclination
    float moonDistLocal_z = config.physics_moon_distance; // Moon along +Z in local orbital plane
    float moonPosY = moonDistLocal_z * std::sin(lunarInclination);
    float moonPosZ = moonDistLocal_z * std::cos(lunarInclination);
    glm::vec3 moonPos = earthPos + glm::vec3(0.0f, moonPosY, moonPosZ);
    
    // Moon velocity: orbital velocity (~1022 m/s) perpendicular to position in the inclined plane
    // In the inclined plane, velocity is along -X when position is along +Z
    const float moonOrbitalSpeed = 1022.0f;
    glm::vec3 moonVelRelative = glm::vec3(-moonOrbitalSpeed, 0.0f, 0.0f);
    glm::vec3 moonVel = earthVel + moonVelRelative;
    
    bodies["moon"] = std::make_unique<Body>(config, logger_, "moon", config.physics_moon_mass, moonPos, moonVel);
    bodies["moon"]->setTrajectory(TrajectoryFactory::createMoonTrajectory(config, logger_));
    
    // Set Earth's trajectory to show its orbit around the Sun
    bodies["earth"]->setTrajectory(TrajectoryFactory::createEarthTrajectory(config, logger_));
    
    // Mars - fourth planet
    // Orbital inclination: 1.850° to ecliptic
    {
        glm::vec3 pos = glm::vec3(config.physics_mars_orbit_radius, 0.0f, 0.0f);
        glm::vec3 vel = glm::vec3(0.0f, 0.0f, config.physics_mars_orbital_velocity);
        bodies["mars"] = std::make_unique<Body>(config, logger_, "mars", config.physics_mars_mass, pos, vel);
        bodies["mars"]->setTrajectory(TrajectoryFactory::createPlanetOrbit(
            config, logger_, config.physics_mars_orbit_radius,
            glm::vec4(0.8f, 0.3f, 0.2f, 0.8f),  // Reddish
            glm::radians(1.850f)
        ));
    }
    
    // Jupiter - fifth planet (gas giant)
    // Orbital inclination: 1.303° to ecliptic
    {
        glm::vec3 pos = glm::vec3(config.physics_jupiter_orbit_radius, 0.0f, 0.0f);
        glm::vec3 vel = glm::vec3(0.0f, 0.0f, config.physics_jupiter_orbital_velocity);
        bodies["jupiter"] = std::make_unique<Body>(config, logger_, "jupiter", config.physics_jupiter_mass, pos, vel);
        bodies["jupiter"]->setTrajectory(TrajectoryFactory::createPlanetOrbit(
            config, logger_, config.physics_jupiter_orbit_radius,
            glm::vec4(0.8f, 0.7f, 0.5f, 0.8f),  // Tan/beige
            glm::radians(1.303f)
        ));
    }
    
    // Saturn - sixth planet (gas giant with rings)
    // Orbital inclination: 2.485° to ecliptic
    {
        glm::vec3 pos = glm::vec3(config.physics_saturn_orbit_radius, 0.0f, 0.0f);
        glm::vec3 vel = glm::vec3(0.0f, 0.0f, config.physics_saturn_orbital_velocity);
        bodies["saturn"] = std::make_unique<Body>(config, logger_, "saturn", config.physics_saturn_mass, pos, vel);
        bodies["saturn"]->setTrajectory(TrajectoryFactory::createPlanetOrbit(
            config, logger_, config.physics_saturn_orbit_radius,
            glm::vec4(0.9f, 0.8f, 0.5f, 0.8f),  // Pale yellow
            glm::radians(2.485f)
        ));
    }
    
    // Uranus - seventh planet (ice giant)
    // Orbital inclination: 0.773° to ecliptic
    {
        glm::vec3 pos = glm::vec3(config.physics_uranus_orbit_radius, 0.0f, 0.0f);
        glm::vec3 vel = glm::vec3(0.0f, 0.0f, config.physics_uranus_orbital_velocity);
        bodies["uranus"] = std::make_unique<Body>(config, logger_, "uranus", config.physics_uranus_mass, pos, vel);
        bodies["uranus"]->setTrajectory(TrajectoryFactory::createPlanetOrbit(
            config, logger_, config.physics_uranus_orbit_radius,
            glm::vec4(0.6f, 0.8f, 0.9f, 0.8f),  // Cyan/light blue
            glm::radians(0.773f)
        ));
    }
    
    // Neptune - eighth planet (ice giant)
    // Orbital inclination: 1.770° to ecliptic
    {
        glm::vec3 pos = glm::vec3(config.physics_neptune_orbit_radius, 0.0f, 0.0f);
        glm::vec3 vel = glm::vec3(0.0f, 0.0f, config.physics_neptune_orbital_velocity);
        bodies["neptune"] = std::make_unique<Body>(config, logger_, "neptune", config.physics_neptune_mass, pos, vel);
        bodies["neptune"]->setTrajectory(TrajectoryFactory::createPlanetOrbit(
            config, logger_, config.physics_neptune_orbit_radius,
            glm::vec4(0.2f, 0.3f, 0.8f, 0.8f),  // Deep blue
            glm::radians(1.770f)
        ));
    }

    if (!bodies["sun"] || !bodies["earth"] || !bodies["moon"]) {
        LOG_ERROR(logger_, "Simulation", "Failed to initialize celestial bodies!");
        return;
    }
    
    // Update rocket initial position relative to Earth
    rocket.setPosition(earthPos + glm::vec3(0.0f, config.physics_earth_radius, 0.0f));
    rocket.setVelocity(earthVel); // Rocket starts with Earth's orbital velocity
    rocket.setEarthPosition(earthPos);  // Set Earth position for altitude calculations

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
    
    // Sun (scaled down for rendering - actual sun is much larger)
    std::vector<GLfloat> sunVertices;
    float sunScale = config.physics_sun_radius / config.physics_earth_radius;
    for (size_t i = 0; i < earthVertices.size(); i += 3) {
        sunVertices.push_back(earthVertices[i] * sunScale);
        sunVertices.push_back(earthVertices[i + 1] * sunScale);
        sunVertices.push_back(earthVertices[i + 2] * sunScale);
    }
    
    try {
        bodies["sun"]->renderObject = std::make_unique<RenderObject>(sunVertices, earthIndices);
        LOG_INFO(logger_, "Simulation", "Sun renderObject created: vertices=" + 
                 std::to_string(sunVertices.size()) + ", indices=" + std::to_string(earthIndices.size()));
    } catch (const std::exception& e) {
        LOG_ERROR(logger_, "Simulation", "Error creating sun renderObject: " + std::string(e.what()));
        return;
    }
    
    // Helper lambda to create scaled planet render objects
    auto createPlanetRenderObject = [&](const std::string& name, float radius) {
        std::vector<GLfloat> vertices;
        float scale = radius / config.physics_earth_radius;
        for (size_t i = 0; i < earthVertices.size(); i += 3) {
            vertices.push_back(earthVertices[i] * scale);
            vertices.push_back(earthVertices[i + 1] * scale);
            vertices.push_back(earthVertices[i + 2] * scale);
        }
        try {
            bodies[name]->renderObject = std::make_unique<RenderObject>(vertices, earthIndices);
            LOG_INFO(logger_, "Simulation", name + " renderObject created");
        } catch (const std::exception& e) {
            LOG_ERROR(logger_, "Simulation", "Error creating " + name + " renderObject: " + std::string(e.what()));
        }
    };
    
    // Create render objects for all planets
    createPlanetRenderObject("mercury", config.physics_mercury_radius);
    createPlanetRenderObject("venus", config.physics_venus_radius);
    createPlanetRenderObject("mars", config.physics_mars_radius);
    createPlanetRenderObject("jupiter", config.physics_jupiter_radius);
    createPlanetRenderObject("saturn", config.physics_saturn_radius);
    createPlanetRenderObject("uranus", config.physics_uranus_radius);
    createPlanetRenderObject("neptune", config.physics_neptune_radius);
    
    // Verify all bodies have render objects
    LOG_INFO(logger_, "Simulation", "=== Render Object Status ===");
    for (const auto& [name, body] : bodies) {
        bool hasRender = body->renderObject != nullptr;
        if (hasRender) {
            LOG_INFO(logger_, "Simulation", name + ": renderObject OK");
        } else {
            LOG_ERROR(logger_, "Simulation", name + ": renderObject MISSING!");
        }
    }
    LOG_INFO(logger_, "Simulation", "============================");
    
    LOG_INFO(logger_, "Simulation", "All 8 planets initialized (Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune)");
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
        
        // Update trajectory for bodies that have one (scale position for rendering)
        body->update(dt);
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
    glm::vec3 target = glm::vec3(0.0f);
    
    // Update Earth position for camera (used in Locked mode to calculate radial direction)
    if (bodies.find("earth") != bodies.end()) {
        camera.setEarthPosition(bodies.at("earth")->position * scale);
    }
    
    // Update camera target based on mode
    if (camera.mode == Camera::Mode::Locked || camera.mode == Camera::Mode::Free) {
        // Use getRenderPosition() to get the same coordinates used for rocket rendering
        target = rocket.getRenderPosition();
    } else if (camera.mode == Camera::Mode::FixedEarth) {
        // Earth center in heliocentric coordinates
        if (bodies.find("earth") != bodies.end()) {
            target = bodies.at("earth")->position * scale;
            camera.setFixedTarget(target);
        }
    } else if (camera.mode == Camera::Mode::FixedMoon) {
        if (bodies.find("moon") != bodies.end()) {
            target = bodies.at("moon")->position * scale;
            camera.setFixedTarget(target);
        }
    } else if (camera.mode == Camera::Mode::Overview) {
        // Midpoint between Earth and Moon
        if (bodies.find("earth") != bodies.end() && bodies.find("moon") != bodies.end()) {
            target = (bodies.at("earth")->position + bodies.at("moon")->position) * scale * 0.5f;
            camera.setFixedTarget(target);
        }
    } else if (camera.mode == Camera::Mode::SolarSystem) {
        // Sun center (origin in heliocentric coords)
        target = glm::vec3(0.0f);
        camera.setFixedTarget(target);
    } else if (camera.mode == Camera::Mode::FullSolarSystem) {
        // Sun center for full solar system view
        target = glm::vec3(0.0f);
        camera.setFixedTarget(target);
    }

    camera.update(target);
    glm::mat4 view = camera.getViewMatrix();
    
    // Adjust near/far planes based on mode for proper rendering
    float nearPlane;
    float farPlane;
    if (camera.mode == Camera::Mode::Locked) {
        nearPlane = 0.01f;  // 10 meters
        farPlane = camera.distance * 100.0f;
    } else if (camera.mode == Camera::Mode::SolarSystem || camera.mode == Camera::Mode::FullSolarSystem) {
        // Solar system scale: need very large far plane
        nearPlane = std::max(1000.0f, camera.distance * 0.0001f);
        farPlane = camera.distance * 10.0f;
    } else {
        nearPlane = std::max(0.1f, camera.distance * 0.001f);
        farPlane = camera.distance * 10.0f;
    }
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / sceneHeight, nearPlane, farPlane);

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    
    // Render the Sun (orange)
    if (bodies.find("sun") != bodies.end() && bodies.at("sun")->renderObject) {
        glm::mat4 sunModel = glm::translate(glm::mat4(1.0f), bodies.at("sun")->position * scale);
        sunModel = glm::scale(sunModel, glm::vec3(scale, scale, scale));
        shader.setMat4("model", sunModel);
        shader.setVec4("color", glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); // Orange
        bodies.at("sun")->renderObject->render();
    }
    
    // Render Earth's orbit (only visible in solar system view)
    if (bodies.find("earth") != bodies.end()) {
        bodies.at("earth")->render(shader);
    }

    // Render the Earth (blue)
    if (bodies.find("earth") != bodies.end() && bodies.at("earth")->renderObject) {
        glm::mat4 earthModel = glm::translate(glm::mat4(1.0f), bodies.at("earth")->position * scale);
        earthModel = glm::scale(earthModel, glm::vec3(scale, scale, scale));
        shader.setMat4("model", earthModel);
        shader.setVec4("color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Blue
        bodies.at("earth")->renderObject->render();
    } else {
        LOG_ERROR(logger_, "Simulation", "Earth is null or has no renderObject!");
    }

    rocket.render(shader);

    // Render the Moon (gray)
    if (bodies.find("moon") != bodies.end() && bodies.at("moon")->renderObject) {
        glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), bodies.at("moon")->position * scale);
        moonModel = glm::scale(moonModel, glm::vec3(scale, scale, scale));
        shader.setMat4("model", moonModel);
        shader.setVec4("color", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f)); // Gray
        bodies.at("moon")->renderObject->render();
        
        // Render Moon's orbit centered on Earth's position
        glm::vec3 earthCenter = bodies.at("earth")->position * scale;
        bodies.at("moon")->render(shader, earthCenter);
    } else {
        LOG_ERROR(logger_, "Simulation", "Moon is null or has no renderObject!");
    }
    
    // Helper lambda to render a planet with its orbit
    auto renderPlanet = [&](const std::string& name, const glm::vec4& color) {
        if (bodies.find(name) != bodies.end()) {
            auto& body = bodies.at(name);
            // Render orbit
            body->render(shader);
            // Render planet sphere
            if (body->renderObject) {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), body->position * scale);
                model = glm::scale(model, glm::vec3(scale, scale, scale));
                shader.setMat4("model", model);
                shader.setVec4("color", color);
                body->renderObject->render();
            }
        }
    };
    
    // Render other planets
    renderPlanet("mercury", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));    // Gray
    renderPlanet("venus", glm::vec4(0.9f, 0.7f, 0.5f, 1.0f));      // Orange-ish
    renderPlanet("mars", glm::vec4(0.8f, 0.3f, 0.2f, 1.0f));       // Reddish
    renderPlanet("jupiter", glm::vec4(0.8f, 0.7f, 0.5f, 1.0f));    // Tan/beige
    renderPlanet("saturn", glm::vec4(0.9f, 0.8f, 0.5f, 1.0f));     // Pale yellow
    renderPlanet("uranus", glm::vec4(0.6f, 0.8f, 0.9f, 1.0f));     // Cyan/light blue
    renderPlanet("neptune", glm::vec4(0.2f, 0.3f, 0.8f, 1.0f));    // Deep blue

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
    // Support much higher time scales for testing orbital mechanics
    // Use multiplicative scaling for large values
    if (delta > 0) {
        // Increasing: multiply by 1.5 when above 100, otherwise add delta
        if (timeScale >= 100.0f) {
            timeScale *= 1.5f;
        } else {
            timeScale += delta;
        }
    } else {
        // Decreasing: divide by 1.5 when above 100, otherwise subtract delta
        if (timeScale > 100.0f) {
            timeScale /= 1.5f;
        } else {
            timeScale += delta;
        }
    }
    // Clamp to reasonable range: 0.1x to 1,000,000x (for testing year-long orbits)
    timeScale = std::max(0.1f, std::min(timeScale, 1000000.0f));
    LOG_INFO(logger_, "Simulation", "Time scale adjusted to " + std::to_string(timeScale));
}

void Simulation::adjustCameraDistance(float delta) { 
    camera.zoom(delta);
    LOG_INFO(logger_, "Simulation", "Camera distance adjusted to " + std::to_string(camera.distance));
}

void Simulation::adjustCameraRotation(float deltaPitch, float deltaYaw) {
    camera.rotate(deltaPitch, deltaYaw);
    LOG_INFO(logger_, "Simulation", "Camera rotation adjusted: pitch=" + std::to_string(camera.pitch) + 
             ", yaw=" + std::to_string(camera.yaw));
}

void Simulation::adjustCameraMode(Camera::Mode mode) {
    camera.setMode(mode);
    
    // Scale factor for rendering (meters to km)
    const float scale = 0.001f;
    
    // Set appropriate target and distance based on mode
    // Note: distance is in rendering units (km), not meters
    switch (mode) {
        case Camera::Mode::FixedEarth:
            if (bodies.find("earth") != bodies.end()) {
                camera.setFixedTarget(bodies.at("earth")->position * scale);
            }
            camera.distance = 20000.0f; // 20,000 km view distance
            break;
        case Camera::Mode::FixedMoon:
            if (bodies.find("moon") != bodies.end()) {
                camera.setFixedTarget(bodies.at("moon")->position * scale);
                camera.distance = 10000.0f; // 10,000 km view distance
            }
            break;
        case Camera::Mode::Overview:
            // Midpoint between Earth and Moon
            if (bodies.find("earth") != bodies.end() && bodies.find("moon") != bodies.end()) {
                glm::vec3 midpoint = (bodies.at("earth")->position + bodies.at("moon")->position) * scale * 0.5f;
                camera.setFixedTarget(midpoint);
                camera.distance = 500000.0f; // 500,000 km to see both
            }
            break;
        case Camera::Mode::SolarSystem:
            // Sun at origin, view inner solar system (up to Mars)
            camera.setFixedTarget(glm::vec3(0.0f));
            camera.distance = 300000000.0f; // 300 million km (~2 AU) to see inner planets
            break;
        case Camera::Mode::FullSolarSystem:
            // Sun at origin, view entire solar system including Neptune (~30 AU)
            camera.setFixedTarget(glm::vec3(0.0f));
            camera.distance = 5000000000.0f; // 5 billion km (~33 AU) to see all 8 planets
            break;
        case Camera::Mode::Locked:
            camera.distance = 500.0f; // 500 km follow distance
            break;
        case Camera::Mode::Free:
            // Keep current settings
            break;
    }
    
    LOG_INFO(logger_, "Simulation", "Camera mode changed to: " + 
             std::string(camera.getModeName()));
}

void Simulation::adjustCameraTarget(const glm::vec3& target) {
    camera.setFixedTarget(target);
    LOG_INFO(logger_, "Simulation", "Camera target adjusted to " + glm::to_string(target));
}

void Simulation::focusOnBody(const std::string& bodyName) {
    const float scale = 0.001f;  // Rendering scale (meters to km)
    
    if (bodyName == "rocket") {
        // Switch to Locked mode for rocket
        camera.setMode(Camera::Mode::Locked);
        LOG_INFO(logger_, "Simulation", "Camera focused on rocket (Locked mode)");
        return;
    }
    
    // Find the body
    auto it = bodies.find(bodyName);
    if (it == bodies.end()) {
        LOG_WARN(logger_, "Simulation", "Body not found: " + bodyName);
        return;
    }
    
    // Get body position
    glm::vec3 bodyPos = it->second->position * scale;
    
    // Get body radius from config (in meters, convert to km)
    float bodyRadiusKm = 0.0f;
    if (bodyName == "sun") bodyRadiusKm = config.physics_sun_radius * scale;
    else if (bodyName == "mercury") bodyRadiusKm = config.physics_mercury_radius * scale;
    else if (bodyName == "venus") bodyRadiusKm = config.physics_venus_radius * scale;
    else if (bodyName == "earth") bodyRadiusKm = config.physics_earth_radius * scale;
    else if (bodyName == "mars") bodyRadiusKm = config.physics_mars_radius * scale;
    else if (bodyName == "jupiter") bodyRadiusKm = config.physics_jupiter_radius * scale;
    else if (bodyName == "saturn") bodyRadiusKm = config.physics_saturn_radius * scale;
    else if (bodyName == "uranus") bodyRadiusKm = config.physics_uranus_radius * scale;
    else if (bodyName == "neptune") bodyRadiusKm = config.physics_neptune_radius * scale;
    else if (bodyName == "moon") bodyRadiusKm = config.physics_moon_radius * scale;
    else bodyRadiusKm = 1000.0f;  // Default 1000 km
    
    // Calculate appropriate viewing distance based on body size
    float viewDistance = bodyRadiusKm * 5.0f;  // 5x radius for good viewing
    
    // Minimum distance for very small bodies
    viewDistance = std::max(viewDistance, 5000.0f);  // At least 5000 km
    
    // Adjust view distance for specific body types
    if (bodyName == "sun") {
        viewDistance = bodyRadiusKm * 8.0f;  // Sun is huge
    } else if (bodyName == "jupiter" || bodyName == "saturn") {
        viewDistance = bodyRadiusKm * 6.0f;  // Gas giants
    }
    
    // Set camera mode based on body type
    if (bodyName == "earth") {
        camera.setMode(Camera::Mode::FixedEarth);
    } else if (bodyName == "moon") {
        camera.setMode(Camera::Mode::FixedMoon);
    } else {
        // For all other bodies (including sun and planets), use SolarSystem mode
        // This mode uses fixedTarget which we'll set to the body position
        camera.setMode(Camera::Mode::SolarSystem);
    }
    
    // Set the camera target and position
    camera.setFixedTarget(bodyPos);
    camera.distance = viewDistance;
    camera.target = bodyPos;  // Also set target directly for immediate effect
    camera.position = bodyPos + glm::vec3(0.0f, viewDistance * 0.3f, viewDistance);  // Position camera
    
    LOG_INFO(logger_, "Simulation", "Camera focused on " + bodyName + 
             " at distance " + std::to_string(viewDistance) + " km");
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

Camera& Simulation::getCamera() {
    return camera;
}

glm::vec3 Simulation::getMoonPos() const {
    return moonPos;
}

const BODY_MAP& Simulation::getBodies() const {
    return bodies;
}