#include <gtest/gtest.h>
#include "app/config.h"
#include <fstream>

void PrintTo(const glm::dvec3& v, std::ostream* os) {
    *os << "glm::dvec3(" << v.x << ", " << v.y << ", " << v.z << ")";
}

TEST(ConfigTest, DefaultValues) {
    Config config;
    EXPECT_DOUBLE_EQ(config.rocket_mass, 501000.0);
    EXPECT_DOUBLE_EQ(config.rocket_thrust, 20000000.0);
    EXPECT_EQ(config.rocket_initial_position.x, 0.0);
    EXPECT_EQ(config.rocket_initial_position.y, 6371000.0);
    EXPECT_EQ(config.rocket_initial_position.z, 0.0);
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
    EXPECT_DOUBLE_EQ(config.rocket_mass, 2000.0);
    EXPECT_DOUBLE_EQ(config.rocket_thrust, 30000000.0);
}

TEST(ConfigTest, LoadFromFileInvalid) {
    Config config;
    // Nonexistent file should throw ConfigError
    EXPECT_THROW(config.loadFromFile("nonexistent_file.json"), ConfigError);
    // Defaults should be retained after failed load
    EXPECT_DOUBLE_EQ(config.rocket_mass, 501000.0);
}

TEST(ConfigTest, LoadFromFileMalformedJson) {
    std::ofstream file("./var/test_malformed.json");
    file << "{ this is not valid json !!!";
    file.close();

    Config config;
    EXPECT_THROW(config.loadFromFile("./var/test_malformed.json"), ConfigError);
    // Defaults should be retained after parse failure
    EXPECT_DOUBLE_EQ(config.rocket_mass, 501000.0);
}

TEST(ConfigTest, MoonParameters) {
    Config config;
    EXPECT_DOUBLE_EQ(config.physics_moon_radius, 1737100.0);
    EXPECT_DOUBLE_EQ(config.physics_moon_mass, 7.34767309e22);
    EXPECT_DOUBLE_EQ(config.physics_moon_distance, 384400000.0);
    EXPECT_DOUBLE_EQ(config.physics_moon_gravity_constant, 6.674e-11);
    EXPECT_DOUBLE_EQ(config.physics_moon_gravity, 1.62);

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
    EXPECT_DOUBLE_EQ(config.physics_moon_radius, 173710.0);
    EXPECT_DOUBLE_EQ(config.physics_moon_mass, 7.34767309e21);
    EXPECT_DOUBLE_EQ(config.physics_moon_distance, 38440000.0);
    EXPECT_DOUBLE_EQ(config.physics_moon_gravity_constant, 6.674e-11);
    EXPECT_DOUBLE_EQ(config.physics_moon_gravity, 1.62);
}

TEST(ConfigTest, CameraParameters) {
    Config config;
    EXPECT_FLOAT_EQ(config.camera_pitch, 45.0f);
    EXPECT_FLOAT_EQ(config.camera_yaw, 45.0f);
    EXPECT_FLOAT_EQ(config.camera_distance, 500000.0f);
    EXPECT_EQ(config.camera_position.x, 0.0f);
    EXPECT_EQ(config.camera_position.y, 6371000.0f);
    EXPECT_EQ(config.camera_position.z, 0.0f);
    
    EXPECT_EQ(config.camera_target.x, 0.0f);
    EXPECT_EQ(config.camera_target.y, 6371000.0f);
    EXPECT_EQ(config.camera_target.z, 0.0f);

    std::ofstream file("./var/test_camera_config.json");
    file << R"({
        "camera": {
            "pitch": 30.0,
            "yaw": 60.0,
            "distance": 1000000.0,
            "position": [100000.0, 200000.0, 300000.0],
            "target": [400000.0, 500000.0, 600000.0]
        }
    })";
    file.close();

    config.loadFromFile("./var/test_camera_config.json");
    EXPECT_FLOAT_EQ(config.camera_pitch, 30.0f);
    EXPECT_FLOAT_EQ(config.camera_yaw, 60.0f);
    EXPECT_FLOAT_EQ(config.camera_distance, 1000000.0f);
    EXPECT_EQ(config.camera_position.x, 100000.0f);
    EXPECT_EQ(config.camera_position.y, 200000.0f);
    EXPECT_EQ(config.camera_position.z, 300000.0f);
    EXPECT_EQ(config.camera_target.x, 400000.0f);
    EXPECT_EQ(config.camera_target.y, 500000.0f);
    EXPECT_EQ(config.camera_target.z, 600000.0f);
}