#include "core/rocket.h"
#include "rendering/render_object.h"
#include "logging/logger.h"
#include "test.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <thread>

using ::testing::_;
using ::testing::AtLeast;

class RocketTest : public ::testing::Test {
protected:
    Config config;
    std::unique_ptr<Rocket> rocket;
    std::shared_ptr<MockLogger> logger;

    std::unique_ptr<MockRenderObject> mockRenderObject;
    std::unique_ptr<MockRenderObject> mockTrajectory;
    std::unique_ptr<MockRenderObject> mockPrediction;

    void SetUp() override {
        config = Config();
        logger = std::make_shared<MockLogger>();
        EXPECT_CALL(*logger, log(_, _, _)).Times(AtLeast(0));
        EXPECT_CALL(*logger, set_level(_)).Times(AtLeast(0));


        mockRenderObject = std::make_unique<MockRenderObject>();
        mockTrajectory = std::make_unique<MockRenderObject>();
        EXPECT_CALL(*mockTrajectory, updateBuffer(_, _, _)).Times(AtLeast(0));
        mockPrediction = std::make_unique<MockRenderObject>();
        EXPECT_CALL(*mockPrediction, updateBuffer(_, _, _)).Times(AtLeast(0));
            
        rocket = std::make_unique<Rocket>(config, logger, FlightPlan(config.flight_plan_path));
    }
};

TEST_F(RocketTest, Initialization) {
    EXPECT_DOUBLE_EQ(rocket->getThrustDirection().x, 0.0);
    EXPECT_DOUBLE_EQ(rocket->getThrustDirection().y, 0.0);
    EXPECT_DOUBLE_EQ(rocket->getThrustDirection().z, 0.0);
    
    rocket->setRender(std::move(mockRenderObject));
    rocket->setTrajectoryRender(std::move(mockTrajectory), std::move(mockPrediction));
    rocket->init();
    EXPECT_DOUBLE_EQ(rocket->getThrustDirection().x, 0.0);
    EXPECT_DOUBLE_EQ(rocket->getThrustDirection().y, 1.0);
    EXPECT_DOUBLE_EQ(rocket->getThrustDirection().z, 0.0);
}

TEST_F(RocketTest, InitInjectsMockRenderObject) {
    auto mockRender = std::make_unique<MockRenderObject>();
    auto mockTrajectory = std::make_unique<MockRenderObject>();
    auto mockPrediction = std::make_unique<MockRenderObject>();

    rocket->setRender(std::move(mockRenderObject));
    rocket->setTrajectoryRender(std::move(mockTrajectory), std::move(mockPrediction));
    EXPECT_NO_THROW(rocket->init());
}

TEST_F(RocketTest, OffsetPosition_Default) {
    config.rocket_initial_position = glm::dvec3(0.0, 6371000.0, 0.0);
    config.simulation_rendering_scale = 0.001f;

    // Recreate rocket with updated config
    rocket = std::make_unique<Rocket>(config, logger, FlightPlan(config.flight_plan_path));

    glm::vec3 offset = rocket->offsetPosition();

    EXPECT_FLOAT_EQ(offset.x, 0.0f);
    EXPECT_FLOAT_EQ(offset.y, 6371.0f);
    EXPECT_FLOAT_EQ(offset.z, 0.0f);
}

TEST_F(RocketTest, OffsetPosition_CustomPosition) {
    config.rocket_initial_position = glm::dvec3(1000.0, 6372000.0, 2000.0);
    config.simulation_rendering_scale = 0.001f;

    // Recreate rocket with updated config
    rocket = std::make_unique<Rocket>(config, logger, FlightPlan(config.flight_plan_path));

    glm::dvec3 customPosition = glm::dvec3(2000.0, 6373000.0, 4000.0);
    glm::vec3 offset = rocket->offsetPosition(customPosition);

    EXPECT_FLOAT_EQ(offset.x, 2.0f);
    EXPECT_FLOAT_EQ(offset.y, 6373.0f);
    EXPECT_FLOAT_EQ(offset.z, 4.0f);
}

TEST_F(RocketTest, FlightPlanExecution) {
    nlohmann::json json = R"(
        {
            "flight_plan": [
                {"condition": {"altitude_min": 0.0, "altitude_max": 10000.0},
                 "action": {"thrust": 25000000.0, "direction": [0.0, 1.0, 0.0]}}
            ]
        }
    )"_json;
    FlightPlan plan(json);
    rocket = std::make_unique<Rocket>(config, logger, plan);

    rocket->setRender(std::move(mockRenderObject));
    rocket->setTrajectoryRender(std::move(mockTrajectory), std::move(mockPrediction));

    rocket->init();
    rocket->launched = true;

    rocket->update(0.1f, {});
    EXPECT_DOUBLE_EQ(rocket->getThrustDirection().y, 1.0);
    EXPECT_DOUBLE_EQ(rocket->thrust, 25000000.0);
}

TEST_F(RocketTest, ConcurrentUpdate) {
    rocket->setRender(std::move(mockRenderObject));
    rocket->setTrajectoryRender(std::move(mockTrajectory), std::move(mockPrediction));

    rocket->init();
    rocket->launched = true;
    std::thread t1([&] { rocket->update(0.1f, {}); });
    std::thread t2([&] { rocket->update(0.1f, {}); });
    t1.join();
    t2.join();
}