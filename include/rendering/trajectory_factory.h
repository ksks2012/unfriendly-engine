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
            config.physics_earth_radius
        };
        return std::make_unique<Trajectory>(trajConfig, logger);
    }

    static std::unique_ptr<Trajectory> createRocketPredictionTrajectory(const Config& config, std::shared_ptr<ILogger> logger) {
        Trajectory::Config trajConfig{
            1000,
            0.1f,
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), // green
            config.simulation_rendering_scale,
            config.physics_earth_radius
        };
        return std::make_unique<Trajectory>(trajConfig, logger);
    }

    static std::unique_ptr<Trajectory> createBodyTrajectory(const Config& config, std::shared_ptr<ILogger> logger) {
        Trajectory::Config trajConfig{
            1000,
            0.1f,
            glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), // blue
            config.simulation_rendering_scale,
            config.physics_earth_radius
        };
        return std::make_unique<Trajectory>(trajConfig, logger);
    }

    // General creation method, supports custom parameters
    static std::unique_ptr<Trajectory> createCustomTrajectory(const Config& config, std::shared_ptr<ILogger> logger, size_t maxPoints, float sampleInterval, const glm::vec4& color) {
        Trajectory::Config trajConfig{
            maxPoints,
            sampleInterval,
            color,
            config.simulation_rendering_scale,
            config.physics_earth_radius
        };
        return std::make_unique<Trajectory>(trajConfig, logger);
    }

    static std::unique_ptr<Trajectory> createMoonTrajectory(const Config& config, std::shared_ptr<ILogger> logger) {
        Trajectory::Config trajConfig{
            1000,
            0.1f,
            // glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), // red
            glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), // gray
            config.simulation_rendering_scale,
            config.physics_earth_radius
        };
        
        auto traj = std::make_unique<Trajectory>(trajConfig, logger);
        traj->init();
    
        const float radius = config.physics_moon_distance * config.simulation_rendering_scale;
        const size_t numPoints = traj->getPoints().size();
        std::vector<glm::vec3> points(numPoints);
        for (size_t i = 0; i < numPoints; ++i) {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / numPoints;
            points[i] = glm::vec3(radius * std::cos(theta + glm::pi<float>() / 2.0f), radius * std::sin(theta + glm::pi<float>() / 2.0f), 0.0f);
            LOG_ORBIT(logger, "Moon", 0.0f, points[i], radius, glm::vec3(0.0f));
        }
        traj->setPoints(points);
    
        return traj;
    }
};

#endif