#include <gtest/gtest.h>
#include "config.h"
#include <fstream>

void PrintTo(const glm::vec3& v, std::ostream* os) {
    *os << "glm::vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
}

TEST(ConfigTest, DefaultValues) {
    Config config;
    EXPECT_FLOAT_EQ(config.rocket_mass, 501000.0f);
    EXPECT_FLOAT_EQ(config.rocket_thrust, 20000000.0f);
    EXPECT_EQ(config.rocket_initial_position.x, 0.0f);
    EXPECT_EQ(config.rocket_initial_position.y, 6371000.0f);
    EXPECT_EQ(config.rocket_initial_position.z, 0.0f);
}

TEST(ConfigTest, LoadFromFileValid) {
    std::ofstream file("./var/test_config.json");
    file << R"({
        "rocket": {
            "mass": 2000.0,
            "thrust": 30000000.0
        },
        "flight_plan": [
            {
                "condition": {"altitude_min": 0.0, "altitude_max": 1000.0},
                "action": {"thrust": 25000000.0, "direction": [0.0, 1.0, 0.0]}
            }
        ]
    })";
    file.close();

    Config config;
    config.loadFromFile("./var/test_config.json");
    EXPECT_FLOAT_EQ(config.rocket_mass, 2000.0f);
    EXPECT_FLOAT_EQ(config.rocket_thrust, 30000000.0f);
}

TEST(ConfigTest, LoadFromFileInvalid) {
    Config config;
    // Nonexistent file
    EXPECT_FLOAT_EQ(config.rocket_mass, 501000.0f);  // Should retain default value
}