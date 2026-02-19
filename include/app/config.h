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
    double rocket_mass = 501000.0;
    double rocket_fuel_mass = 500000.0;
    double rocket_thrust = 20000000.0;
    double rocket_exhaust_velocity = 3000.0;
    glm::dvec3 rocket_initial_position = {0.0, 6371000.0, 0.0};
    glm::dvec3 rocket_initial_velocity = {0.0, 0.0, 0.0};
    float rocket_rotation_speed = 90.0f;
    float rocket_direction_cooldown = 0.01f;
    std::string flight_plan_path = "etc/flight_plan.json";

    // Physics parameters
    // TODO: setting structure of stars
    // Sun parameters
    double physics_sun_radius = 696340000.0;           // 696,340 km
    double physics_sun_mass = 1.989e30;                // kg
    double physics_earth_orbit_radius = 149597870700.0; // 1 AU in meters
    double physics_earth_orbital_velocity = 29780.0;   // m/s
    
    // Mercury parameters
    double physics_mercury_radius = 2439700.0;         // 2,439.7 km
    double physics_mercury_mass = 3.3011e23;           // kg
    double physics_mercury_orbit_radius = 57909050000.0; // 0.387 AU in meters
    double physics_mercury_orbital_velocity = 47362.0; // m/s
    
    // Venus parameters
    double physics_venus_radius = 6051800.0;           // 6,051.8 km
    double physics_venus_mass = 4.8675e24;             // kg
    double physics_venus_orbit_radius = 108208000000.0; // 0.723 AU in meters
    double physics_venus_orbital_velocity = 35020.0;   // m/s
    
    // Mars parameters
    double physics_mars_radius = 3389500.0;            // 3,389.5 km
    double physics_mars_mass = 6.4171e23;              // kg
    double physics_mars_orbit_radius = 227939200000.0; // 1.524 AU in meters
    double physics_mars_orbital_velocity = 24077.0;    // m/s
    
    // Jupiter parameters
    double physics_jupiter_radius = 69911000.0;        // 69,911 km
    double physics_jupiter_mass = 1.8982e27;           // kg
    double physics_jupiter_orbit_radius = 778.57e9;    // 5.204 AU in meters
    double physics_jupiter_orbital_velocity = 13070.0; // m/s
    
    // Saturn parameters
    double physics_saturn_radius = 58232000.0;         // 58,232 km
    double physics_saturn_mass = 5.6834e26;            // kg
    double physics_saturn_orbit_radius = 1433.53e9;    // 9.583 AU in meters
    double physics_saturn_orbital_velocity = 9680.0;   // m/s
    
    // Uranus parameters
    double physics_uranus_radius = 25362000.0;         // 25,362 km
    double physics_uranus_mass = 8.6810e25;            // kg
    double physics_uranus_orbit_radius = 2872.46e9;    // 19.19 AU in meters
    double physics_uranus_orbital_velocity = 6800.0;   // m/s
    
    // Neptune parameters
    double physics_neptune_radius = 24622000.0;        // 24,622 km
    double physics_neptune_mass = 1.02413e26;          // kg
    double physics_neptune_orbit_radius = 4495.06e9;   // 30.07 AU in meters
    double physics_neptune_orbital_velocity = 5430.0;  // m/s
    
    // Earth parameters
    double physics_earth_radius = 6371000.0;
    double physics_gravity_constant = 6.674e-11;
    double physics_earth_mass = 5.972e24;
    double physics_air_density = 1.225;
    double physics_scale_height = 8000.0;
    double physics_drag_coefficient = 0.1;
    double physics_cross_section_area = 0.5;
    // Moon parameters
    double physics_moon_radius = 1737100.0;
    double physics_moon_mass = 7.34767309e22;
    double physics_moon_distance = 384400000.0;
    double physics_moon_gravity_constant = 6.674e-11;
    double physics_moon_gravity = 1.62;
    double physics_moon_angular_speed = 2.6617e-6; // radians per second
    double physics_moon_rotation_speed = 2.6617e-6; // radians per second
    double physics_moon_rotation_period = 27.3 * 24.0 * 3600.0; // seconds
    
    // Simulation parameters
    float simulation_trajectory_sample_time = 0.5f;      // Sample every 0.5 seconds
    size_t simulation_trajectory_max_points = 5000;      // Maximum trajectory points
    size_t simulation_prediction_max_points = 500;       // Maximum prediction points
    float simulation_prediction_duration = 30.0f;
    float simulation_prediction_step = 0.1f;
    float simulation_rendering_scale = 0.001f;
    
    // Trajectory colors (RGBA)
    glm::vec4 trajectory_rocket_color = {1.0f, 0.0f, 0.0f, 1.0f};      // Red
    glm::vec4 trajectory_prediction_color = {0.0f, 1.0f, 0.0f, 0.7f};  // Green with transparency
    glm::vec4 trajectory_moon_color = {0.5f, 0.5f, 0.5f, 0.8f};        // Gray
    glm::vec4 trajectory_earth_color = {0.0f, 0.5f, 1.0f, 0.8f};       // Light blue

    // Logger settings
    int logger_level = 3; // 0: DEBUG, 1: INFO, 2: WARN, 3: ERROR

    // Camera settings
    float camera_pitch = 45.0f;
    float camera_yaw = 45.0f;
    float camera_distance = 500000.0f;
    glm::vec3 camera_position = {0.0f, 6371000.0f, 0.0f};
    glm::vec3 camera_target = {0.0f, 6371000.0f, 0.0f};
    
    // Camera mode distances (in km for rendering)
    float camera_distance_locked = 500.0f;               // Follow rocket distance
    float camera_distance_earth = 20000.0f;              // Earth view distance
    float camera_distance_moon = 10000.0f;               // Moon view distance
    float camera_distance_overview = 500000.0f;          // Earth-Moon overview
    float camera_distance_solar_system = 300000000.0f;   // Inner solar system (~2 AU)
    float camera_distance_full_solar = 5000000000.0f;    // Full solar system (~33 AU)
    float camera_min_focus_distance = 5000.0f;           // Minimum focus distance

private:
    void setDefaults();
    void parseConfig(const json&);
};

#endif