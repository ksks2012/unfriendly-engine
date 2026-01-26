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
    // Sun parameters
    float physics_sun_radius = 696340000.0f;           // 696,340 km
    float physics_sun_mass = 1.989e30f;                // kg
    float physics_earth_orbit_radius = 149597870700.0f; // 1 AU in meters
    float physics_earth_orbital_velocity = 29780.0f;   // m/s
    
    // Mercury parameters
    float physics_mercury_radius = 2439700.0f;         // 2,439.7 km
    float physics_mercury_mass = 3.3011e23f;           // kg
    float physics_mercury_orbit_radius = 57909050000.0f; // 0.387 AU in meters
    float physics_mercury_orbital_velocity = 47362.0f; // m/s
    
    // Venus parameters
    float physics_venus_radius = 6051800.0f;           // 6,051.8 km
    float physics_venus_mass = 4.8675e24f;             // kg
    float physics_venus_orbit_radius = 108208000000.0f; // 0.723 AU in meters
    float physics_venus_orbital_velocity = 35020.0f;   // m/s
    
    // Mars parameters
    float physics_mars_radius = 3389500.0f;            // 3,389.5 km
    float physics_mars_mass = 6.4171e23f;              // kg
    float physics_mars_orbit_radius = 227939200000.0f; // 1.524 AU in meters
    float physics_mars_orbital_velocity = 24077.0f;    // m/s
    
    // Jupiter parameters
    float physics_jupiter_radius = 69911000.0f;        // 69,911 km
    float physics_jupiter_mass = 1.8982e27f;           // kg
    float physics_jupiter_orbit_radius = 778.57e9f;    // 5.204 AU in meters
    float physics_jupiter_orbital_velocity = 13070.0f; // m/s
    
    // Saturn parameters
    float physics_saturn_radius = 58232000.0f;         // 58,232 km
    float physics_saturn_mass = 5.6834e26f;            // kg
    float physics_saturn_orbit_radius = 1433.53e9f;    // 9.583 AU in meters
    float physics_saturn_orbital_velocity = 9680.0f;   // m/s
    
    // Uranus parameters
    float physics_uranus_radius = 25362000.0f;         // 25,362 km
    float physics_uranus_mass = 8.6810e25f;            // kg
    float physics_uranus_orbit_radius = 2872.46e9f;    // 19.19 AU in meters
    float physics_uranus_orbital_velocity = 6800.0f;   // m/s
    
    // Neptune parameters
    float physics_neptune_radius = 24622000.0f;        // 24,622 km
    float physics_neptune_mass = 1.02413e26f;          // kg
    float physics_neptune_orbit_radius = 4495.06e9f;   // 30.07 AU in meters
    float physics_neptune_orbital_velocity = 5430.0f;  // m/s
    
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
    float physics_moon_gravity_constant = 6.674e-11f;
    float physics_moon_gravity = 1.62f;
    float physics_moon_angular_speed = 2.6617e-6f; // radians per second
    float physics_moon_rotation_speed = 2.6617e-6f; // radians per second
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