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
            100,
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
};

#endif