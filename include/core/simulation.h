#ifndef SIMULATION_H
#define SIMULATION_H

#define GLM_ENABLE_EXPERIMENTAL

#include "body.h"
#include "rendering/camera.h"
#include "logging/logger.h"
#include "core/flight_plan.h"
#include "core/octree.h"
#include "rendering/render_object.h"
#include "rendering/saturn_rings.h"
#include "core/rocket.h"
#include "rendering/shader.h"
#include "logging/spdlog_logger.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <memory>
#include <string>

class Simulation {
public:
    Simulation(Camera &camera);
    explicit Simulation(Config& config, std::shared_ptr<ILogger> logger, Camera &camera);
    ~Simulation();

    void init();
    void update(float deltaTime);
    void render(const Shader& shader) const;

    void setTimeScale(float ts);
    void adjustTimeScale(float delta);
    void adjustCameraDistance(float delta);
    void adjustCameraRotation(float deltaPitch, float deltaYaw); // Adjust camera rotation
    void adjustCameraMode(Camera::Mode mode); // Adjust camera mode
    void adjustCameraTarget(const glm::vec3& target); // Adjust camera target    
    void focusOnBody(const std::string& bodyName);  // Focus camera on a specific body

    glm::dvec3 computeBodyAcceleration(const Body& body, const BODY_MAP& bodies) const; // Velocity Verlet


    float getTimeScale() const;
    Rocket& getRocket();
    Camera& getCamera();

    glm::dvec3 getMoonPos() const;
    const BODY_MAP& getBodies() const;
    float getRenderScale() const;  // Get rendering scale factor
    const glm::dvec3& getRenderOrigin() const { return renderOrigin_; }
    
    // Get projection and view matrices for UI rendering
    void getRenderMatrices(int width, int height, glm::mat4& projection, glm::mat4& view) const;

private:
    Config config;
    Rocket rocket;
    BODY_MAP bodies;

    Camera& camera;

    float timeScale;

    void updateCameraPosition() const; // Update camera position

    glm::dvec3 moonPos;
    std::unique_ptr<RenderObject> mapEarth;
    std::unique_ptr<RenderObject> mapMoon;
    std::unique_ptr<RenderObject> mapRocket;
    
    // Saturn's rings
    std::unique_ptr<SaturnRings> saturnRings_;

    // Camera-relative rendering origin (double precision)
    // All render positions are computed relative to this point to avoid float precision loss
    mutable glm::dvec3 renderOrigin_;

    // Barnes-Hut octree for O(n log n) gravitational force calculation
    Octree octree_;

    // Build octree from current body state (call once per frame)
    void buildOctree();

    std::shared_ptr<ILogger> logger_;
};

#endif