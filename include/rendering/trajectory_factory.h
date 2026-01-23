#ifndef TRAJECTORY_FACTORY_H
#define TRAJECTORY_FACTORY_H

#include "rendering/trajectory.h"
#include "app/config.h"

#include <memory>

class TrajectoryFactory {
public:
    static std::unique_ptr<Trajectory> createRocketTrajectory(const Config& config, std::shared_ptr<ILogger> logger) {
        Trajectory::Config trajConfig{
            1000,
            0.1f,
            glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), // red
            config.simulation_rendering_scale,
            config.physics_earth_radius,
            Trajectory::RenderMode::LineStrip  // Open path for rocket trail
        };
        return std::make_unique<Trajectory>(trajConfig, logger);
    }

    static std::unique_ptr<Trajectory> createRocketPredictionTrajectory(const Config& config, std::shared_ptr<ILogger> logger) {
        Trajectory::Config trajConfig{
            500,  // Fewer points for prediction
            0.2f, // Larger interval for prediction
            glm::vec4(0.0f, 1.0f, 0.0f, 0.7f), // green with transparency
            config.simulation_rendering_scale,
            config.physics_earth_radius,
            Trajectory::RenderMode::LineStrip  // Open path for prediction
        };
        return std::make_unique<Trajectory>(trajConfig, logger);
    }

    static std::unique_ptr<Trajectory> createBodyTrajectory(const Config& config, std::shared_ptr<ILogger> logger) {
        Trajectory::Config trajConfig{
            1000,
            0.1f,
            glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), // blue
            config.simulation_rendering_scale,
            config.physics_earth_radius,
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
            config.physics_earth_radius,
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
            glm::vec4(0.5f, 0.5f, 0.5f, 0.8f), // gray with slight transparency
            config.simulation_rendering_scale,
            config.physics_earth_radius,
            Trajectory::RenderMode::LineLoop  // Closed loop for orbit
        };
        
        auto traj = std::make_unique<Trajectory>(trajConfig, logger);
        traj->init();
    
        const float radius = config.physics_moon_distance * config.simulation_rendering_scale;
        std::vector<glm::vec3> points(orbitPoints);
        for (size_t i = 0; i < orbitPoints; ++i) {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / orbitPoints;
            // Moon orbit is in the X-Y plane
            points[i] = glm::vec3(
                radius * std::cos(theta), 
                radius * std::sin(theta), 
                0.0f
            );
        }
        traj->setPoints(points);
    
        return traj;
    }
};

#endif