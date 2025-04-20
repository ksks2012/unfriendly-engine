#ifndef CONFIG_H
#define CONFIG_H

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>
#include <string>

using json = nlohmann::json;

class Config {
public:
    Config();

    void loadFromFile(const std::string&);

public:
    // Rocket parameters
    float rocket_mass = 501000.0f;
    float rocket_fuel_mass = 500000.0f;
    float rocket_thrust = 20000000.0f;
    float rocket_exhaust_velocity = 3000.0f;
    glm::vec3 rocket_initial_position = {0.0f, 6371000.0f, 0.0f};
    glm::vec3 rocket_initial_velocity = {0.0f, 0.0f, 0.0f};
    float rocket_rotation_speed = 90.0f;
    float rocket_direction_cooldown = 0.01f;
    std::string flight_plan_path = "etc/flight_plan.json";

    // Physics parameters
    // TODO: setting structure of stars
    // Earth parameters
    float physics_earth_radius = 6371000.0f;
    float physics_gravity_constant = 6.674e-11f;
    float physics_earth_mass = 5.972e24f;
    float physics_air_density = 1.225f;
    float physics_scale_height = 8000.0f;
    float physics_drag_coefficient = 0.1f;
    float physics_cross_section_area = 0.5f;
    // Moon parameters
    float physics_moon_radius = 1737100.0f;
    float physics_moon_mass = 7.34767309e22f;
    float physics_moon_distance = 384400000.0f;
    float physics_moon_gravity_constant = 1.982e-14f;
    float physics_moon_gravity = 1.62f;
    float physics_moon_angular_speed = 2.665e-6f; // radians per second
    float physics_moon_rotation_speed = 2.0f * M_PI / 27.3f; // radians per second
    float physics_moon_rotation_period = 27.3f * 24.0f * 3600.0f; // seconds
    
    // Simulation parameters
    float simulation_trajectory_sample_time = 0.1f;
    float simulation_prediction_duration = 30.0f;
    float simulation_prediction_step = 0.1f;
    float simulation_rendering_scale = 0.001f;

    // Logger settings
    int logger_level = 3; // 0: DEBUG, 1: INFO, 2: WARN, 3: ERROR

    // Camera settings
    float camera_pitch = 45.0f;
    float camera_yaw = 45.0f;
    float camera_distance = 500000.0f;
    glm::vec3 camera_position = {0.0f, 6371000.0f, 0.0f};
    glm::vec3 camera_target = {0.0f, 6371000.0f, 0.0f};

private:
    void setDefaults();
    void parseConfig(const json&);
};

#endif