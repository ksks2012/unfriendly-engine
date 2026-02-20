#ifndef CONFIG_H
#define CONFIG_H

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

using json = nlohmann::json;

// Configuration for a single planet (orbit, mass, radius, rendering)
struct PlanetConfig {
    std::string name;
    double radius;              // meters
    double mass;                // kg
    double orbit_radius;        // meters (semi-major axis)
    double orbital_velocity;    // m/s
    float orbital_inclination;  // radians (relative to ecliptic)
    glm::vec4 orbit_color;      // RGBA for orbit line rendering
    float view_multiplier;      // Camera distance multiplier for focus mode
};

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
    
    // Planet parameters (Mercury through Neptune)
    // Indexed by name for lookup; order matches solar system distance from Sun
    std::vector<PlanetConfig> planets;
    
    // Lookup a planet by name (returns nullptr if not found)
    const PlanetConfig* getPlanet(const std::string& name) const {
        auto it = planetIndex_.find(name);
        return (it != planetIndex_.end()) ? &planets[it->second] : nullptr;
    }
    
    // Convenience: get planet radius by name (returns 0 if not found)
    double getPlanetRadius(const std::string& name) const {
        const auto* p = getPlanet(name);
        return p ? p->radius : 0.0;
    }
    
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
    
    // Map planet name -> index in planets vector (rebuilt after loading)
    std::unordered_map<std::string, size_t> planetIndex_;
    void buildPlanetIndex();
};

#endif