#include <gtest/gtest.h>
#include "app/config.h"
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

TEST(ConfigTest, MoonParameters) {
    Config config;
    EXPECT_FLOAT_EQ(config.physics_moon_radius, 1737100.0f);
    EXPECT_FLOAT_EQ(config.physics_moon_mass, 7.34767309e22f);
    EXPECT_FLOAT_EQ(config.physics_moon_distance, 384400000.0f);
    EXPECT_FLOAT_EQ(config.physics_moon_gravity_constant, 1.982e-14f);
    EXPECT_FLOAT_EQ(config.physics_moon_gravity, 1.62f);

    std::ofstream file("./var/test_moon_config.json");
    file << R"({
        "physics": {
            "moon_radius": 173710.0,
            "moon_mass": 7.34767309e21,
            "moon_distance": 38440000.0
        }
    })";
    file.close();

    config.loadFromFile("./var/test_moon_config.json");
    EXPECT_FLOAT_EQ(config.physics_moon_radius, 173710.0f);
    EXPECT_FLOAT_EQ(config.physics_moon_mass, 7.34767309e21f);
    EXPECT_FLOAT_EQ(config.physics_moon_distance, 38440000.0f);
    EXPECT_FLOAT_EQ(config.physics_moon_gravity_constant, 1.982e-14f);
    EXPECT_FLOAT_EQ(config.physics_moon_gravity, 1.62f);
}