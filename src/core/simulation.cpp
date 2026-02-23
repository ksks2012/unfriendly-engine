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
        camera(camera), moonPos(0.0, 384400000.0, 0.0) {
}

Simulation::Simulation(Config& config, std::shared_ptr<ILogger> logger, Camera& camera) : 
        config(config), rocket(config, logger, FlightPlan(config.flight_plan_path)), 
        logger_(logger), camera(camera), timeScale(1.0f), moonPos(0.0, 384400000.0, 0.0) {
    if (!logger_) {
        throw std::runtime_error("Logger is null");
    }
    logger_->set_level((LogLevel)config.logger_level);
}

Simulation::~Simulation() = default;

void Simulation::init() {
    LOG_DEBUG(logger_, "Simulation", "Initializing simulation...");
    
    // Sun at origin (heliocentric coordinate system)
    bodies["sun"] = std::make_unique<Body>(config, logger_, "sun", config.physics_sun_mass, glm::dvec3(0.0), glm::dvec3(0.0));
    
    // Initialize all planets from config data
    // Earth and Moon have special handling; other planets use the generic loop
    const auto* earthCfg = config.getPlanet("earth");
    glm::dvec3 earthPos = glm::dvec3(earthCfg->orbit_radius, 0.0, 0.0);
    glm::dvec3 earthVel = glm::dvec3(0.0, 0.0, earthCfg->orbital_velocity);
    
    for (const auto& planet : config.planets) {
        if (planet.name == "earth" || planet.name == "moon") continue;
        
        glm::dvec3 pos = glm::dvec3(planet.orbit_radius, 0.0, 0.0);
        glm::dvec3 vel = glm::dvec3(0.0, 0.0, planet.orbital_velocity);
        bodies[planet.name] = std::make_unique<Body>(config, logger_, planet.name, planet.mass, pos, vel);
        bodies[planet.name]->setTrajectory(TrajectoryFactory::createPlanetOrbit(
            config, logger_, planet.orbit_radius, planet.orbit_color, planet.orbital_inclination
        ));
    }
    
    // Earth
    bodies["earth"] = std::make_unique<Body>(config, logger_, "earth", config.physics_earth_mass, earthPos, earthVel);
    bodies["earth"]->setTrajectory(TrajectoryFactory::createEarthTrajectory(config, logger_));
    
    // Moon orbiting Earth with ~5.145° orbital inclination relative to the ecliptic
    const float lunarInclination = glm::radians(5.145f);
    
    // Moon position in the inclined orbital plane
    double moonDistLocal_z = config.physics_moon_distance;
    double moonPosY = moonDistLocal_z * std::sin(lunarInclination);
    double moonPosZ = moonDistLocal_z * std::cos(lunarInclination);
    glm::dvec3 moonPos = earthPos + glm::dvec3(0.0, moonPosY, moonPosZ);
    
    // Moon velocity: orbital velocity (~1022 m/s) perpendicular to position in the inclined plane
    const double moonOrbitalSpeed = 1022.0;
    glm::dvec3 moonVelRelative = glm::dvec3(-moonOrbitalSpeed, 0.0, 0.0);
    glm::dvec3 moonVel = earthVel + moonVelRelative;
    
    bodies["moon"] = std::make_unique<Body>(config, logger_, "moon", config.physics_moon_mass, moonPos, moonVel);
    bodies["moon"]->setTrajectory(TrajectoryFactory::createMoonTrajectory(config, logger_));

    if (!bodies["sun"] || !bodies["earth"] || !bodies["moon"]) {
        LOG_ERROR(logger_, "Simulation", "Failed to initialize celestial bodies!");
        return;
    }
    
    // Update rocket initial position relative to Earth
    rocket.setPosition(earthPos + glm::dvec3(0.0, config.physics_earth_radius, 0.0));
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
        bodies["earth"]->renderer.setSphereRenderObject(std::make_unique<RenderObject>(earthVertices, earthIndices));
        LOG_INFO(logger_, "Simulation", "Earth sphere created: vertices=" + 
                 std::to_string(earthVertices.size()) + ", indices=" + std::to_string(earthIndices.size()));
    } catch (const std::exception& e) {
        LOG_ERROR(logger_, "Simulation", "Error creating earth sphere: " + std::string(e.what()));
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
        bodies["moon"]->renderer.setSphereRenderObject(std::make_unique<RenderObject>(moonVertices, earthIndices));
        LOG_INFO(logger_, "Simulation", "Moon sphere created: vertices=" + 
                 std::to_string(moonVertices.size()) + ", indices=" + std::to_string(earthIndices.size()));
    } catch (const std::exception& e) {
        LOG_ERROR(logger_, "Simulation", "Error creating moon sphere: " + std::string(e.what()));
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
        bodies["sun"]->renderer.setSphereRenderObject(std::make_unique<RenderObject>(sunVertices, earthIndices));
        LOG_INFO(logger_, "Simulation", "Sun sphere created: vertices=" + 
                 std::to_string(sunVertices.size()) + ", indices=" + std::to_string(earthIndices.size()));
    } catch (const std::exception& e) {
        LOG_ERROR(logger_, "Simulation", "Error creating sun sphere: " + std::string(e.what()));
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
            bodies[name]->renderer.setSphereRenderObject(std::make_unique<RenderObject>(vertices, earthIndices));
            LOG_INFO(logger_, "Simulation", name + " sphere created");
        } catch (const std::exception& e) {
            LOG_ERROR(logger_, "Simulation", "Error creating " + name + " sphere: " + std::string(e.what()));
        }
    };
    
    // Create render objects for all planets
    for (const auto& planet : config.planets) {
        if (planet.name == "earth") continue;  // Earth already has its own render object
        createPlanetRenderObject(planet.name, planet.radius);
    }
    
    // Verify all bodies have render objects
    LOG_INFO(logger_, "Simulation", "=== Render Object Status ===");
    for (const auto& [name, body] : bodies) {
        bool hasRender = body->renderer.hasSphere();
        if (hasRender) {
            LOG_INFO(logger_, "Simulation", name + ": sphere OK");
        } else {
            LOG_ERROR(logger_, "Simulation", name + ": sphere MISSING!");
        }
    }
    LOG_INFO(logger_, "Simulation", "============================");
    
    // Initialize Saturn's rings
    saturnRings_ = std::make_unique<SaturnRings>(config.getPlanet("saturn")->radius);
    saturnRings_->init();
    LOG_INFO(logger_, "Simulation", "Saturn's rings initialized");
    
    LOG_INFO(logger_, "Simulation", "All 8 planets initialized (Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune)");
    LOG_INFO(logger_, "Simulation", "Map objects initialized");
}

void Simulation::update(float deltaTime) {
    static float elapsed_time = 0.0f;
    elapsed_time += deltaTime * timeScale;
    double dt = static_cast<double>(deltaTime * timeScale);
    
    // Build Barnes-Hut octree once per frame for O(n log n) gravity
    buildOctree();
    
    // Step 1: Update all celestial bodies (Velocity Verlet)
    std::vector<std::pair<std::string, glm::dvec3>> current_accs;
    for (const auto& [name, body] : bodies) {
        glm::dvec3 acc = computeBodyAcceleration(*body, bodies);
        current_accs.emplace_back(name, acc);
    }
    
    // Update positions
    for (const auto& [name, body] : bodies) {
        body->position += body->velocity * dt + 0.5 * current_accs[std::distance(bodies.begin(), bodies.find(name))].second * dt * dt;
    }
    
    // Rebuild octree after position update for accurate new accelerations
    buildOctree();
    
    // Calculate new accelerations and update velocities
    for (auto& [name, body] : bodies) {
        glm::dvec3 new_acc = computeBodyAcceleration(*body, bodies);
        
        body->velocity += 0.5 * (current_accs[std::distance(bodies.begin(), bodies.find(name))].second + new_acc) * dt;
        
        // Check for NaN
        if (std::isnan(body->position.x) || std::isnan(body->velocity.x)) {
            LOG_ERROR(logger_, "Simulation", "NaN detected in " + name + ": Pos=" + 
                      glm::to_string(glm::vec3(body->position)) + ", Vel=" + glm::to_string(glm::vec3(body->velocity)));
        }
        
        // Update trajectory for bodies that have one (scale position for rendering)
        body->update(dt);
    }
    
    rocket.update(dt, bodies, &octree_);
    
    double moon_radius = glm::length(bodies["moon"]->position);
    LOG_ORBIT(logger_, "Moon", elapsed_time, glm::vec3(bodies["moon"]->position), static_cast<float>(moon_radius), glm::vec3(bodies["moon"]->velocity));
    LOG_DEBUG(logger_, "Simulation", "Rocket: Pos=" + glm::to_string(glm::vec3(rocket.getPosition())));
}

void Simulation::updateCameraPosition() const {
    // TODO: Follow the rocket
}

void Simulation::render(const Shader& shader) const {
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    float sceneHeight = height * 0.8f;

    // Scale factor (physical unit meters to rendering unit kilometers)
    const double scale = static_cast<double>(config.simulation_rendering_scale);
    const float scalef = config.simulation_rendering_scale;

    // ---------------------------------------------------------------
    // Camera-relative rendering (origin rebasing)
    // ---------------------------------------------------------------
    // Problem: In heliocentric coordinates, positions are ~1.5e11 m.
    // After scale (×0.001), that's ~1.5e8 km.  float32 has only ~7
    // significant digits, so camera offsets of 10,000 km are lost.
    // Solution: Choose a renderOrigin (dvec3) near the camera focus,
    // compute all render positions as vec3((pos - renderOrigin) * scale).
    // This keeps float values small and precise.
    // ---------------------------------------------------------------

    // Determine renderOrigin based on camera mode (in physics coords, meters)
    if (camera.mode == Camera::Mode::Locked || camera.mode == Camera::Mode::Free) {
        renderOrigin_ = rocket.getPosition();
    } else if (camera.mode == Camera::Mode::FixedEarth) {
        if (bodies.find("earth") != bodies.end())
            renderOrigin_ = bodies.at("earth")->position;
        else
            renderOrigin_ = glm::dvec3(0.0);
    } else if (camera.mode == Camera::Mode::FixedMoon) {
        if (bodies.find("moon") != bodies.end())
            renderOrigin_ = bodies.at("moon")->position;
        else
            renderOrigin_ = glm::dvec3(0.0);
    } else if (camera.mode == Camera::Mode::Overview) {
        if (bodies.find("earth") != bodies.end() && bodies.find("moon") != bodies.end())
            renderOrigin_ = (bodies.at("earth")->position + bodies.at("moon")->position) * 0.5;
        else
            renderOrigin_ = glm::dvec3(0.0);
    } else if (camera.mode == Camera::Mode::SolarSystem || camera.mode == Camera::Mode::FullSolarSystem) {
        renderOrigin_ = glm::dvec3(0.0);  // Sun is at origin — no rebasing needed
    } else if (camera.mode == Camera::Mode::FocusBody) {
        const std::string& bodyName = camera.focusBodyName;
        if (!bodyName.empty() && bodies.find(bodyName) != bodies.end())
            renderOrigin_ = bodies.at(bodyName)->position;
        else
            renderOrigin_ = glm::dvec3(0.0);
    } else {
        renderOrigin_ = glm::dvec3(0.0);
    }

    // Helper: convert physics position to origin-relative render position
    auto toRender = [&](const glm::dvec3& physPos) -> glm::vec3 {
        return glm::vec3((physPos - renderOrigin_) * scale);
    };

    glm::vec3 target = glm::vec3(0.0f);

    // Update Earth position for camera (used in Locked mode to calculate radial direction)
    if (bodies.find("earth") != bodies.end()) {
        camera.setEarthPosition(toRender(bodies.at("earth")->position));
    }
    
    // Update camera target based on mode (all positions are origin-relative)
    if (camera.mode == Camera::Mode::Locked || camera.mode == Camera::Mode::Free) {
        target = toRender(rocket.getPosition());
    } else if (camera.mode == Camera::Mode::FixedEarth) {
        if (bodies.find("earth") != bodies.end()) {
            target = toRender(bodies.at("earth")->position);
            camera.setFixedTarget(target);
        }
    } else if (camera.mode == Camera::Mode::FixedMoon) {
        if (bodies.find("moon") != bodies.end()) {
            target = toRender(bodies.at("moon")->position);
            camera.setFixedTarget(target);
        }
    } else if (camera.mode == Camera::Mode::Overview) {
        if (bodies.find("earth") != bodies.end() && bodies.find("moon") != bodies.end()) {
            glm::dvec3 midpoint = (bodies.at("earth")->position + bodies.at("moon")->position) * 0.5;
            target = toRender(midpoint);
            camera.setFixedTarget(target);
        }
    } else if (camera.mode == Camera::Mode::SolarSystem) {
        target = toRender(glm::dvec3(0.0));
        camera.setFixedTarget(target);
    } else if (camera.mode == Camera::Mode::FullSolarSystem) {
        target = toRender(glm::dvec3(0.0));
        camera.setFixedTarget(target);
    } else if (camera.mode == Camera::Mode::FocusBody) {
        const std::string& bodyName = camera.focusBodyName;
        if (!bodyName.empty() && bodies.find(bodyName) != bodies.end()) {
            target = toRender(bodies.at(bodyName)->position);
            camera.setFixedTarget(target);
        }
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
    if (bodies.find("sun") != bodies.end() && bodies.at("sun")->renderer.hasSphere()) {
        glm::mat4 sunModel = glm::translate(glm::mat4(1.0f), toRender(bodies.at("sun")->position));
        sunModel = glm::scale(sunModel, glm::vec3(scalef, scalef, scalef));
        shader.setMat4("model", sunModel);
        shader.setVec4("color", glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); // Orange
        bodies.at("sun")->renderer.renderSphere();
    }
    
    // Render Earth's orbit (only visible in solar system view)
    if (bodies.find("earth") != bodies.end()) {
        bodies.at("earth")->render(shader);
    }

    // Render the Earth (blue)
    if (bodies.find("earth") != bodies.end() && bodies.at("earth")->renderer.hasSphere()) {
        glm::mat4 earthModel = glm::translate(glm::mat4(1.0f), toRender(bodies.at("earth")->position));
        earthModel = glm::scale(earthModel, glm::vec3(scalef, scalef, scalef));
        shader.setMat4("model", earthModel);
        shader.setVec4("color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Blue
        bodies.at("earth")->renderer.renderSphere();
    } else {
        LOG_ERROR(logger_, "Simulation", "Earth is null or has no sphere!");
    }

    rocket.render(shader, renderOrigin_);

    // Render the Moon (gray)
    if (bodies.find("moon") != bodies.end() && bodies.at("moon")->renderer.hasSphere()) {
        glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), toRender(bodies.at("moon")->position));
        moonModel = glm::scale(moonModel, glm::vec3(scalef, scalef, scalef));
        shader.setMat4("model", moonModel);
        shader.setVec4("color", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f)); // Gray
        bodies.at("moon")->renderer.renderSphere();
        
        // Render Moon's orbit centered on Earth's position (origin-relative)
        glm::vec3 earthCenter = toRender(bodies.at("earth")->position);
        bodies.at("moon")->render(shader, earthCenter);
    } else {
        LOG_ERROR(logger_, "Simulation", "Moon is null or has no sphere!");
    }
    
    // Helper lambda to render a planet with its orbit
    auto renderPlanet = [&](const std::string& name, const glm::vec4& color) {
        if (bodies.find(name) != bodies.end()) {
            auto& body = bodies.at(name);
            // Render orbit
            body->render(shader);
            // Render planet sphere
            if (body->renderer.hasSphere()) {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), toRender(body->position));
                model = glm::scale(model, glm::vec3(scalef, scalef, scalef));
                shader.setMat4("model", model);
                shader.setVec4("color", color);
                body->renderer.renderSphere();
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
    
    // Render Saturn's rings (after Saturn's sphere, with proper blending)
    if (saturnRings_ && bodies.find("saturn") != bodies.end()) {
        glm::mat4 saturnModel = glm::translate(glm::mat4(1.0f), toRender(bodies.at("saturn")->position));
        saturnRings_->render(saturnModel, view, projection, scalef);
        // Restore main shader after ring rendering
        shader.use();
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
    const double scale = static_cast<double>(config.simulation_rendering_scale);

    // Update renderOrigin FIRST so that toRender() uses the correct origin
    // for the new mode (not the stale origin from the previous mode).
    switch (mode) {
        case Camera::Mode::Locked:
        case Camera::Mode::Free:
            renderOrigin_ = rocket.getPosition();
            break;
        case Camera::Mode::FixedEarth:
            if (bodies.find("earth") != bodies.end())
                renderOrigin_ = bodies.at("earth")->position;
            break;
        case Camera::Mode::FixedMoon:
            if (bodies.find("moon") != bodies.end())
                renderOrigin_ = bodies.at("moon")->position;
            break;
        case Camera::Mode::Overview:
            if (bodies.find("earth") != bodies.end() && bodies.find("moon") != bodies.end())
                renderOrigin_ = (bodies.at("earth")->position + bodies.at("moon")->position) * 0.5;
            break;
        case Camera::Mode::SolarSystem:
        case Camera::Mode::FullSolarSystem:
            renderOrigin_ = glm::dvec3(0.0);
            break;
        case Camera::Mode::FocusBody: {
            const std::string& bodyName = camera.focusBodyName;
            if (!bodyName.empty() && bodies.find(bodyName) != bodies.end())
                renderOrigin_ = bodies.at(bodyName)->position;
            break;
        }
    }

    // Helper: convert physics position to origin-relative render position
    auto toRender = [&](const glm::dvec3& physPos) -> glm::vec3 {
        return glm::vec3((physPos - renderOrigin_) * scale);
    };
    
    // Set appropriate target and distance based on mode
    // Distances are loaded from config for easy customization
    switch (mode) {
        case Camera::Mode::FixedEarth:
            if (bodies.find("earth") != bodies.end()) {
                camera.setFixedTarget(toRender(bodies.at("earth")->position));
            }
            camera.distance = config.camera_distance_earth;
            break;
        case Camera::Mode::FixedMoon:
            if (bodies.find("moon") != bodies.end()) {
                camera.setFixedTarget(toRender(bodies.at("moon")->position));
                camera.distance = config.camera_distance_moon;
            }
            break;
        case Camera::Mode::Overview:
            // Midpoint between Earth and Moon
            if (bodies.find("earth") != bodies.end() && bodies.find("moon") != bodies.end()) {
                glm::dvec3 midpoint = (bodies.at("earth")->position + bodies.at("moon")->position) * 0.5;
                camera.setFixedTarget(toRender(midpoint));
                camera.distance = config.camera_distance_overview;
            }
            break;
        case Camera::Mode::SolarSystem:
            // Sun at origin, view inner solar system (up to Mars)
            camera.setFixedTarget(toRender(glm::dvec3(0.0)));
            camera.distance = config.camera_distance_solar_system;
            break;
        case Camera::Mode::FullSolarSystem:
            // Sun at origin, view entire solar system including Neptune (~30 AU)
            camera.setFixedTarget(toRender(glm::dvec3(0.0)));
            camera.distance = config.camera_distance_full_solar;
            break;
        case Camera::Mode::Locked:
            camera.distance = config.camera_distance_locked;
            break;
        case Camera::Mode::Free:
        case Camera::Mode::FocusBody:
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
    const float scale = config.simulation_rendering_scale;
    const double scaled = static_cast<double>(scale);
    
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
    
    // Update renderOrigin to the focused body's position for float precision
    renderOrigin_ = it->second->position;

    // Body position relative to renderOrigin (will be near zero)
    glm::vec3 bodyPos = glm::vec3((it->second->position - renderOrigin_) * scaled);
    
    // Get body radius from config (in meters, convert to km for rendering)
    float bodyRadiusKm = 0.0f;
    float viewMultiplier = 5.0f;  // Default view distance multiplier
    
    if (bodyName == "sun") {
        bodyRadiusKm = config.physics_sun_radius * scale;
        viewMultiplier = 10.0f;
    } else if (bodyName == "moon") {
        bodyRadiusKm = config.physics_moon_radius * scale;
        viewMultiplier = 10.0f;
    } else {
        // Look up planet from config data
        const auto* planet = config.getPlanet(bodyName);
        if (planet) {
            bodyRadiusKm = planet->radius * scale;
            viewMultiplier = planet->view_multiplier;
        } else {
            bodyRadiusKm = config.physics_earth_radius * scale;  // Default to Earth radius
        }
    }
    
    // Calculate view distance based on body radius and multiplier
    float viewDistance = bodyRadiusKm * viewMultiplier;
    
    // Ensure minimum distance for very small bodies (from config)
    float minDistance = config.camera_min_focus_distance;
    if (bodyName == "moon" || bodyName == "mercury" || bodyName == "mars") {
        minDistance = config.camera_distance_moon;  // Small bodies need more relative distance
    }
    viewDistance = std::max(viewDistance, minDistance);
    
    // Set camera mode - use FocusBody for all bodies selected via UI
    // This allows the camera to track the moving body
    camera.setMode(Camera::Mode::FocusBody);
    camera.focusBodyName = bodyName;  // Store the body name for tracking
    
    // Set the camera target and position
    camera.setFixedTarget(bodyPos);
    camera.distance = viewDistance;
    camera.target = bodyPos;  // Set target directly for immediate effect
    
    // Position camera at an angle for better view
    float camAngle = glm::radians(30.0f);  // 30 degree angle above the ecliptic
    camera.position = bodyPos + glm::vec3(
        viewDistance * std::sin(camAngle) * 0.7f,
        viewDistance * std::sin(camAngle),
        viewDistance * std::cos(camAngle)
    );
    
    // Reset pitch/yaw for consistent view
    camera.pitch = 20.0f;
    camera.yaw = 45.0f;
    
    LOG_INFO(logger_, "Simulation", "Camera focused on " + bodyName + 
             " (radius: " + std::to_string(bodyRadiusKm) + " km, distance: " + 
             std::to_string(viewDistance) + " km)");
}

void Simulation::buildOctree() {
    std::vector<OctreeBody> octreeBodies;
    octreeBodies.reserve(bodies.size());
    for (const auto& [name, body] : bodies) {
        octreeBodies.emplace_back(body->position, body->mass, name);
    }
    octree_.build(octreeBodies);
}

glm::dvec3 Simulation::computeBodyAcceleration(const Body& body, const BODY_MAP& bodies) const {
    // Use direct summation for celestial bodies (only ~10 bodies, O(n²) is trivial).
    // Barnes-Hut octree is reserved for rocket gravity calculations where the
    // number of gravitational sources justifies the O(n log n) approach.
    //
    // Direct summation avoids a subtle octree aliasing bug: when two bodies
    // share similar coordinates (e.g., Earth and Moon have identical X values),
    // internal node center-of-mass can land extremely close to one body,
    // producing a near-zero denominator and catastrophic force blow-up.
    glm::dvec3 acc(0.0);
    for (const auto& [name, other] : bodies) {
        if (&(*other) == &body) continue;  // Skip self
        glm::dvec3 delta = other->position - body.position;
        double distSq = glm::dot(delta, delta);
        double dist = std::sqrt(distSq);
        if (dist < 1.0) continue;  // Softening: skip if < 1 meter
        double distCubed = distSq * dist;
        acc += (config.physics_gravity_constant * other->mass / distCubed) * delta;
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

glm::dvec3 Simulation::getMoonPos() const {
    // Return real-time moon position from physics bodies (dvec3)
    auto it = bodies.find("moon");
    if (it != bodies.end()) {
        return it->second->position;
    }
    return moonPos;  // Fallback to initial value
}

const BODY_MAP& Simulation::getBodies() const {
    return bodies;
}

float Simulation::getRenderScale() const {
    return config.simulation_rendering_scale;
}

void Simulation::getRenderMatrices(int width, int height, glm::mat4& projection, glm::mat4& view) const {
    float sceneHeight = height * 0.8f;
    
    // Calculate near/far planes based on camera mode
    float nearPlane;
    float farPlane;
    if (camera.mode == Camera::Mode::Locked) {
        nearPlane = 0.01f;
        farPlane = camera.distance * 100.0f;
    } else if (camera.mode == Camera::Mode::SolarSystem || camera.mode == Camera::Mode::FullSolarSystem) {
        nearPlane = std::max(1000.0f, camera.distance * 0.0001f);
        farPlane = camera.distance * 10.0f;
    } else {
        nearPlane = std::max(0.1f, camera.distance * 0.001f);
        farPlane = camera.distance * 10.0f;
    }
    
    projection = glm::perspective(glm::radians(45.0f), (float)width / sceneHeight, nearPlane, farPlane);
    view = camera.getViewMatrix();
}