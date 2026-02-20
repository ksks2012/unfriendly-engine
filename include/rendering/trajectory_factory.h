#ifndef TRAJECTORY_FACTORY_H
#define TRAJECTORY_FACTORY_H

#include "rendering/trajectory.h"
#include "app/config.h"

#include <memory>

class TrajectoryFactory {
public:
    static std::unique_ptr<Trajectory> createRocketTrajectory(const Config& config, std::shared_ptr<ILogger> logger) {
        // Use config values for customizable trajectory length and color
        Trajectory::Config trajConfig{
            config.simulation_trajectory_max_points,
            config.simulation_trajectory_sample_time,
            config.trajectory_rocket_color,
            config.simulation_rendering_scale,
            static_cast<float>(config.physics_earth_radius),
            Trajectory::RenderMode::LineStrip  // Open path for rocket trail
        };
        return std::make_unique<Trajectory>(trajConfig, logger);
    }

    static std::unique_ptr<Trajectory> createRocketPredictionTrajectory(const Config& config, std::shared_ptr<ILogger> logger) {
        // Use config values for prediction trajectory
        Trajectory::Config trajConfig{
            config.simulation_prediction_max_points,
            config.simulation_prediction_step,
            config.trajectory_prediction_color,
            config.simulation_rendering_scale,
            static_cast<float>(config.physics_earth_radius),
            Trajectory::RenderMode::LineStrip  // Open path for prediction
        };
        return std::make_unique<Trajectory>(trajConfig, logger);
    }

    static std::unique_ptr<Trajectory> createBodyTrajectory(const Config& config, std::shared_ptr<ILogger> logger) {
        Trajectory::Config trajConfig{
            config.simulation_trajectory_max_points,
            config.simulation_trajectory_sample_time,
            glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), // blue (generic body)
            config.simulation_rendering_scale,
            static_cast<float>(config.physics_earth_radius),
            Trajectory::RenderMode::LineStrip
        };
        return std::make_unique<Trajectory>(trajConfig, logger);
    }

    // General creation method, supports custom parameters
    static std::unique_ptr<Trajectory> createCustomTrajectory(const Config& config, std::shared_ptr<ILogger> logger, 
            size_t maxPoints, float sampleInterval, const glm::vec4& color, 
            Trajectory::RenderMode renderMode = Trajectory::RenderMode::LineStrip) {
        Trajectory::Config trajConfig{
            maxPoints,
            sampleInterval,
            color,
            config.simulation_rendering_scale,
            static_cast<float>(config.physics_earth_radius),
            renderMode
        };
        return std::make_unique<Trajectory>(trajConfig, logger);
    }

    static std::unique_ptr<Trajectory> createMoonTrajectory(const Config& config, std::shared_ptr<ILogger> logger) {
        // Use more points for smoother orbit and LINE_LOOP for closed orbit
        const size_t orbitPoints = 360; // One point per degree for smooth circle
        
        Trajectory::Config trajConfig{
            orbitPoints,
            0.1f,
            config.trajectory_moon_color,
            config.simulation_rendering_scale,
            static_cast<float>(config.physics_earth_radius),
            Trajectory::RenderMode::LineLoop,  // Closed loop for orbit
            true  // isStatic: pre-calculated orbit, don't update dynamically
        };
        
        auto traj = std::make_unique<Trajectory>(trajConfig, logger);
        traj->init();
    
        const float radius = static_cast<float>(config.physics_moon_distance) * config.simulation_rendering_scale;
        
        // Lunar orbital inclination relative to the ecliptic (Earth's orbital plane)
        // The Moon's orbit is inclined about 5.145° from the ecliptic
        const float lunarInclination = glm::radians(5.145f);
        
        std::vector<glm::vec3> points(orbitPoints);
        for (size_t i = 0; i < orbitPoints; ++i) {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / orbitPoints;
            // Moon orbit in its own plane (X-Z), then rotated by inclination around X-axis
            // This tilts the orbit relative to the ecliptic (X-Z plane where Y=0)
            float x = radius * std::cos(theta);
            float z_local = radius * std::sin(theta);
            
            // Apply inclination rotation around X-axis
            float y = z_local * std::sin(lunarInclination);
            float z = z_local * std::cos(lunarInclination);
            
            points[i] = glm::vec3(x, y, z);
        }
        traj->setPoints(points);
    
        return traj;
    }
    
    static std::unique_ptr<Trajectory> createEarthTrajectory(const Config& config, std::shared_ptr<ILogger> logger) {
        // Earth orbit around the Sun (1 AU radius)
        const size_t orbitPoints = 360;
        
        Trajectory::Config trajConfig{
            orbitPoints,
            0.1f,
            config.trajectory_earth_color,
            config.simulation_rendering_scale,
            static_cast<float>(config.physics_earth_radius),
            Trajectory::RenderMode::LineLoop,  // Closed loop for orbit
            true  // isStatic: pre-calculated orbit, don't update dynamically
        };
        
        auto traj = std::make_unique<Trajectory>(trajConfig, logger);
        traj->init();
        
        const float radius = static_cast<float>(config.getPlanet("earth")->orbit_radius) * config.simulation_rendering_scale;
        std::vector<glm::vec3> points(orbitPoints);
        for (size_t i = 0; i < orbitPoints; ++i) {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / orbitPoints;
            // Earth orbit is in the X-Z plane (around the Sun at origin)
            points[i] = glm::vec3(
                radius * std::cos(theta),
                0.0f,
                radius * std::sin(theta)
            );
        }
        traj->setPoints(points);
        
        return traj;
    }
    
    // Generic planet orbit factory function
    // orbitRadius: orbital radius in meters
    // color: orbit line color
    // inclination: orbital inclination in radians (relative to ecliptic, default 0)
    static std::unique_ptr<Trajectory> createPlanetOrbit(
        const Config& config, 
        std::shared_ptr<ILogger> logger,
        float orbitRadius,
        glm::vec4 color,
        float inclination = 0.0f
    ) {
        const size_t orbitPoints = 360;
        
        Trajectory::Config trajConfig{
            orbitPoints,
            0.1f,
            color,
            config.simulation_rendering_scale,
            static_cast<float>(config.physics_earth_radius),
            Trajectory::RenderMode::LineLoop,
            true  // isStatic
        };
        
        auto traj = std::make_unique<Trajectory>(trajConfig, logger);
        traj->init();
        
        const float radius = orbitRadius * config.simulation_rendering_scale;
        std::vector<glm::vec3> points(orbitPoints);
        for (size_t i = 0; i < orbitPoints; ++i) {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / orbitPoints;
            // Base orbit in X-Z plane, then apply inclination
            float x = radius * std::cos(theta);
            float z_local = radius * std::sin(theta);
            
            // Apply inclination rotation around X-axis
            float y = z_local * std::sin(inclination);
            float z = z_local * std::cos(inclination);
            
            points[i] = glm::vec3(x, y, z);
        }
        traj->setPoints(points);
        
        return traj;
    }
};

#endif