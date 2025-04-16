// #include "mock.h"
#include "render_object.h"
#include "rocket.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class MockRenderObject : public IRenderObject {
public:
    // Using legacy Google Mock syntax
    MOCK_CONST_METHOD0(render, void());
    MOCK_CONST_METHOD0(getVao, GLuint());
    MOCK_CONST_METHOD0(getVbo, GLuint());
    MOCK_CONST_METHOD3(renderTrajectory, void(size_t, size_t, size_t));
    MOCK_METHOD3(updateBuffer, void(GLintptr, GLsizei, const void*));
};

class RocketTest : public ::testing::Test {
  protected:
    Config config;
    Rocket *rocket;

    void SetUp() override
    {
        config = Config();
        rocket = new Rocket(config, FlightPlan(config.flight_plan_path));
    }

    void TearDown() override { delete rocket; }
};

TEST_F(RocketTest, Initialization)
{
    EXPECT_FLOAT_EQ(rocket->getThrustDirection().x, 0.0f);
    EXPECT_FLOAT_EQ(rocket->getThrustDirection().y, 0.0f);
    EXPECT_FLOAT_EQ(rocket->getThrustDirection().z, 0.0f);

    auto mockRender = std::make_unique<MockRenderObject>();
    auto mockTrajectory = std::make_unique<MockRenderObject>();
    auto mockPrediction = std::make_unique<MockRenderObject>();

    rocket->setRenderObjects(std::move(mockRender), std::move(mockTrajectory),
                             std::move(mockPrediction));

    rocket->init();
    EXPECT_FLOAT_EQ(rocket->getThrustDirection().x, 0.0f);
    EXPECT_FLOAT_EQ(rocket->getThrustDirection().y, 1.0f);
    EXPECT_FLOAT_EQ(rocket->getThrustDirection().z, 0.0f);
}

TEST_F(RocketTest, InitInjectsMockRenderObject)
{
    auto mockRender = std::make_unique<MockRenderObject>();
    auto mockTrajectory = std::make_unique<MockRenderObject>();
    auto mockPrediction = std::make_unique<MockRenderObject>();

    rocket->setRenderObjects(std::move(mockRender), std::move(mockTrajectory),
                             std::move(mockPrediction));
    EXPECT_NO_THROW(rocket->init());
}

TEST_F(RocketTest, OffsetPosition_Default)
{
    config.rocket_initial_position =
        glm::vec3(0.0f, 6371000.0f, 0.0f);      // Earth's radius in meters
    config.simulation_rendering_scale = 0.001f; // Scale for rendering

    glm::vec3 offset = rocket->offsetPosition();

    EXPECT_FLOAT_EQ(offset.x, 0.0f);
    EXPECT_FLOAT_EQ(offset.y, 6371.0f); // Earth's radius scaled
    EXPECT_FLOAT_EQ(offset.z, 0.0f);
}

TEST_F(RocketTest, OffsetPosition_CustomPosition)
{
    config.rocket_initial_position =
        glm::vec3(1000.0f, 6372000.0f, 2000.0f); // Custom position
    config.simulation_rendering_scale = 0.001f;  // Scale for rendering

    glm::vec3 customPosition = glm::vec3(2000.0f, 6373000.0f, 4000.0f);
    glm::vec3 offset = rocket->offsetPosition(customPosition);

    EXPECT_FLOAT_EQ(offset.x, 2.0f);    // Scaled x
    EXPECT_FLOAT_EQ(offset.y, 6373.0f); // Scaled altitude
    EXPECT_FLOAT_EQ(offset.z, 4.0f);    // Scaled z
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
    rocket = new Rocket(config, plan);

    auto mockRender = std::make_unique<MockRenderObject>();
    auto mockTrajectory = std::make_unique<MockRenderObject>();
    auto mockPrediction = std::make_unique<MockRenderObject>();

    auto *mockTrajectoryRaw = mockTrajectory.get();
    EXPECT_CALL(*mockTrajectoryRaw, updateBuffer(testing::_, testing::_, testing::_)).Times(testing::AtLeast(0));

    rocket->setRenderObjects(std::move(mockRender), std::move(mockTrajectory),
                             std::move(mockPrediction));

    rocket->init();
    rocket->launched = true;

    rocket->trajectorySampleTime = -999.0f; // ignore trajectory
    rocket->update(0.1f, BODY_MAP());

    EXPECT_FLOAT_EQ(rocket->getThrustDirection().y, 1.0f);
    EXPECT_FLOAT_EQ(rocket->thrust, 25000000.0f);
}