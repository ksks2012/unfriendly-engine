#include "logging/logger.h"
#include "rendering/trajectory_factory.h"
#include "test.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class TrajectoryTest : public ::testing::Test {
protected:
    Config config;
    std::shared_ptr<MockLogger> logger;
    std::unique_ptr<MockRenderObject> mockRenderObject;

    void SetUp() override {
        config = Config();
        logger = std::make_shared<MockLogger>();
        EXPECT_CALL(*logger, log(_, _, _)).Times(AtLeast(0));
        EXPECT_CALL(*logger, set_level(_)).Times(AtLeast(0));

        mockRenderObject = std::make_unique<MockRenderObject>();
        EXPECT_CALL(*mockRenderObject, updateBuffer(_, _, _)).Times(AtLeast(0));
    }
};

TEST_F(TrajectoryTest, Constructor) {
    Trajectory::Config trajConfig;
    trajConfig.maxPoints = 100;
    trajConfig.sampleInterval = 0.1f;
    trajConfig.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    trajConfig.scale = 1.0f;
    trajConfig.earthRadius = 6371000.0f;

    auto traj = std::make_unique<Trajectory>(trajConfig, logger);
    EXPECT_EQ(traj->getPoints().size(), 0);
    EXPECT_EQ(traj->getSampleTimer(), 0.0f);
}

TEST_F(TrajectoryTest, TrajectoryFactory) {
    auto traj = TrajectoryFactory::createRocketTrajectory(config, logger);
    EXPECT_EQ(traj->getPoints().size(), 0);
    EXPECT_EQ(traj->getSampleTimer(), 0.0f);    
}

TEST_F(TrajectoryTest, Init) {
    auto traj = TrajectoryFactory::createRocketTrajectory(config, logger);
    traj->setRenderObject(std::move(mockRenderObject));
    EXPECT_NO_THROW(traj->init());
    EXPECT_FALSE(traj->getPoints().empty());
}
    
TEST_F(TrajectoryTest, UpdateValid) {
    auto traj = TrajectoryFactory::createRocketTrajectory(config, logger);
    traj->setRenderObject(std::move(mockRenderObject));
    traj->init();
    // deltaTime must be >= sampleInterval (default 0.5s) to trigger a sample
    traj->update(glm::vec3(0.0f, 6371000.0f, 0.0f), config.simulation_trajectory_sample_time);
    EXPECT_FALSE(traj->getPoints().empty());
    EXPECT_EQ(traj->getSampleTimer(), 0.0f);
    EXPECT_EQ(traj->getPoints().size(), config.simulation_trajectory_max_points);
    EXPECT_EQ(traj->getPoints()[0], glm::vec3(0.0f, 6371000.0f, 0.0f));
}

TEST_F(TrajectoryTest, Reset) {
    auto traj = TrajectoryFactory::createRocketTrajectory(config, logger);
    traj->setRenderObject(std::move(mockRenderObject));
    traj->init();
    traj->update(glm::vec3(0.0f, 6371000.0f, 0.0f), config.simulation_trajectory_sample_time);
    traj->reset();
    EXPECT_EQ(traj->getPoints().size(), config.simulation_trajectory_max_points);
    EXPECT_EQ(traj->getSampleTimer(), 0.0f);
}