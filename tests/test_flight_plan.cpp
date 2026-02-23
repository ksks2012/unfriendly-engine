#include <gtest/gtest.h>
#include "core/flight_plan.h"
#include <fstream>

TEST(FlightPlanTest, LoadFromFileInvalid) {
    // Nonexistent file should throw FlightPlanError
    EXPECT_THROW(FlightPlan("nonexistent_flight_plan.json"), FlightPlanError);
}

TEST(FlightPlanTest, LoadFromFileMalformedJson) {
    std::ofstream file("./var/test_malformed_flight_plan.json");
    file << "{ not valid json at all !!!";
    file.close();

    EXPECT_THROW(FlightPlan("./var/test_malformed_flight_plan.json"), FlightPlanError);
}

TEST(FlightPlanTest, LoadFromFileValid) {
    std::ofstream file("./var/test_flight_plan.json");
    file << R"({
        "flight_plan": [
            {
                "condition": {"altitude_min": 0.0, "altitude_max": 1000.0},
                "action": {"thrust": 25000000.0, "direction": [0.0, 1.0, 0.0]}
            }
        ]
    })";
    file.close();

    FlightPlan plan("./var/test_flight_plan.json");
    ASSERT_EQ(plan.getStages().size(), 1);
    EXPECT_DOUBLE_EQ(plan.getStages()[0].action.thrust, 25000000.0);
}

TEST(FlightPlanTest, ConstructFromJson) {
    nlohmann::json j = nlohmann::json::parse(R"({
        "flight_plan": [
            {
                "condition": {"altitude_min": 100.0, "altitude_max": 500.0},
                "action": {"thrust": 10000000.0, "direction": [1.0, 0.0, 0.0]}
            },
            {
                "condition": {"altitude_min": 500.0, "altitude_max": 2000.0},
                "action": {"thrust": 5000000.0, "direction": [0.0, 1.0, 0.0]}
            }
        ]
    })");

    FlightPlan plan(j);
    ASSERT_EQ(plan.getStages().size(), 2);
    EXPECT_DOUBLE_EQ(plan.getStages()[0].condition.altitude_min, 100.0);
    EXPECT_DOUBLE_EQ(plan.getStages()[1].action.thrust, 5000000.0);
}

TEST(FlightPlanTest, DefaultConstructor) {
    FlightPlan plan;
    EXPECT_TRUE(plan.getStages().empty());
}
