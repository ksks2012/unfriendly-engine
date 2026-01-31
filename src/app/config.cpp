#include "app/config.h"

Config::Config(){
    setDefaults();
}

void Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << filename << ". Using defaults." << std::endl;
        return;
    }
    json config;
    file >> config;
    parseConfig(config);
}

// private

void Config::setDefaults() {
    // Rocket parameters    
    rocket_mass = 501000.0f;
    rocket_fuel_mass = 500000.0f;
    rocket_thrust = 20000000.0f;
    rocket_exhaust_velocity = 3000.0f;
    rocket_initial_position = {0.0f, 6371000.0f, 0.0f};
    rocket_initial_velocity = {0.0f, 0.0f, 0.0f};
    rocket_rotation_speed = 360.0f;
    rocket_direction_cooldown = 0.05f;
    flight_plan_path = "etc/flight_plan.json";

    // Physics parameters
    // Earth parameters
    physics_earth_radius = 6371000.0f;
    physics_gravity_constant = 6.674e-11f;
    physics_earth_mass = 5.972e24f;
    physics_air_density = 1.225f;
    physics_scale_height = 8000.0f;
    physics_drag_coefficient = 0.13;
    physics_cross_section_area = 1.0f;
    // Moon parameters
    physics_moon_radius = 1737100.0f;
    physics_moon_mass = 7.34767309e22f;
    physics_moon_distance = 384400000.0f;
    physics_moon_gravity_constant = 6.674e-11f; // Same as universal gravitational constant
    physics_moon_gravity = 1.62f; // Surface gravity in m/s^2
    physics_moon_angular_speed = 2.6617e-6f; // radians per second (2π / 27.3 days)
    physics_moon_rotation_speed = 2.6617e-6f; // radians per second, tidally locked
    physics_moon_rotation_period = 27.3f * 24.0f * 3600.0f; // seconds (27.3 days)

    // Simulation parameters
    simulation_trajectory_sample_time = 0.5f;
    simulation_trajectory_max_points = 5000;
    simulation_prediction_max_points = 500;
    simulation_prediction_duration = 30.0f;
    simulation_prediction_step = 0.1f;
    simulation_rendering_scale = 0.001f;
    
    // Trajectory colors
    trajectory_rocket_color = {1.0f, 0.0f, 0.0f, 1.0f};
    trajectory_prediction_color = {0.0f, 1.0f, 0.0f, 0.7f};
    trajectory_moon_color = {0.5f, 0.5f, 0.5f, 0.8f};
    trajectory_earth_color = {0.0f, 0.5f, 1.0f, 0.8f};

    // Logger settings
    logger_level = 3; // 0: DEBUG, 1: INFO, 2: WARN, 3: ERROR

    // Camera settings
    camera_pitch = 45.0f;
    camera_yaw = 45.0f;
    camera_distance = 500000.0f;
    camera_position = {0.0f, 6371000.0f, 0.0f};
    camera_target = {0.0f, 6371000.0f, 0.0f};
    
    // Camera mode distances
    camera_distance_locked = 500.0f;
    camera_distance_earth = 20000.0f;
    camera_distance_moon = 10000.0f;
    camera_distance_overview = 500000.0f;
    camera_distance_solar_system = 300000000.0f;
    camera_distance_full_solar = 5000000000.0f;
    camera_min_focus_distance = 5000.0f;
}

void Config::parseConfig(const json& config) {
    // Rocket parameters
    if (config.contains("rocket")) {
        const auto& rocket = config["rocket"];
        rocket_mass = rocket.value("mass", rocket_mass);
        rocket_fuel_mass = rocket.value("fuel_mass", rocket_fuel_mass);
        rocket_thrust = rocket.value("thrust", rocket_thrust);
        rocket_exhaust_velocity = rocket.value("exhaust_velocity", rocket_exhaust_velocity);
        if (rocket.contains("initial_position") && rocket["initial_position"].size() == 3) {
            rocket_initial_position = {
                rocket["initial_position"][0].get<float>(),
                rocket["initial_position"][1].get<float>(),
                rocket["initial_position"][2].get<float>()
            };
        }
        if (rocket.contains("initial_velocity") && rocket["initial_velocity"].size() == 3) {
            rocket_initial_velocity = {
                rocket["initial_velocity"][0].get<float>(),
                rocket["initial_velocity"][1].get<float>(),
                rocket["initial_velocity"][2].get<float>()
            };
        }
        rocket_rotation_speed = rocket.value("rotation_speed", rocket_rotation_speed);
        rocket_direction_cooldown = rocket.value("direction_cooldown", rocket_direction_cooldown);
        flight_plan_path = rocket.value("flight_plan_path", flight_plan_path);
    }

    // Physics parameters
    if (config.contains("physics")) {
        const auto& physics = config["physics"];
        // Earth parameters
        physics_earth_radius = physics.value("earth_radius", physics_earth_radius);
        physics_gravity_constant = physics.value("gravity_constant", physics_gravity_constant);
        physics_earth_mass = physics.value("earth_mass", physics_earth_mass);
        physics_air_density = physics.value("air_density", physics_air_density);
        physics_scale_height = physics.value("scale_height", physics_scale_height);
        physics_drag_coefficient = physics.value("drag_coefficient", physics_drag_coefficient);
        physics_cross_section_area = physics.value("cross_section_area", physics_cross_section_area);
        // Moon parameters
        physics_moon_radius = physics.value("moon_radius", physics_moon_radius);
        physics_moon_mass = physics.value("moon_mass", physics_moon_mass);
        physics_moon_distance = physics.value("moon_distance", physics_moon_distance);
        physics_moon_gravity_constant = physics.value("moon_gravity_constant", physics_moon_gravity_constant);
        physics_moon_gravity = physics.value("moon_gravity", physics_moon_gravity);
        physics_moon_rotation_speed = physics.value("moon_rotation_speed", physics_moon_rotation_speed);
        physics_moon_rotation_period = physics.value("moon_rotation_period", physics_moon_rotation_period);
    }

    // Simulation parameters
    if (config.contains("simulation")) {
        const auto& simulation = config["simulation"];
        simulation_trajectory_sample_time = simulation.value("trajectory_sample_time", simulation_trajectory_sample_time);
        simulation_trajectory_max_points = simulation.value("trajectory_max_points", simulation_trajectory_max_points);
        simulation_prediction_max_points = simulation.value("prediction_max_points", simulation_prediction_max_points);
        simulation_prediction_duration = simulation.value("prediction_duration", simulation_prediction_duration);
        simulation_prediction_step = simulation.value("prediction_step", simulation_prediction_step);
        simulation_rendering_scale = simulation.value("rendering_scale", simulation_rendering_scale);
    }
    
    // Trajectory colors
    if (config.contains("trajectory")) {
        const auto& traj = config["trajectory"];
        auto parseColor = [](const json& arr, glm::vec4 defaultVal) -> glm::vec4 {
            if (arr.is_array() && arr.size() >= 4) {
                return {arr[0].get<float>(), arr[1].get<float>(), 
                        arr[2].get<float>(), arr[3].get<float>()};
            }
            return defaultVal;
        };
        if (traj.contains("rocket_color")) {
            trajectory_rocket_color = parseColor(traj["rocket_color"], trajectory_rocket_color);
        }
        if (traj.contains("prediction_color")) {
            trajectory_prediction_color = parseColor(traj["prediction_color"], trajectory_prediction_color);
        }
        if (traj.contains("moon_color")) {
            trajectory_moon_color = parseColor(traj["moon_color"], trajectory_moon_color);
        }
        if (traj.contains("earth_color")) {
            trajectory_earth_color = parseColor(traj["earth_color"], trajectory_earth_color);
        }
    }

    // Logger settings
    if (config.contains("logger")) {
        const auto& logger = config["logger"];
        logger_level = logger.value("level", logger_level);
    }

    // Camera settings
    if (config.contains("camera")) {
        const auto& camera = config["camera"];
        camera_pitch = camera.value("pitch", camera_pitch);
        camera_yaw = camera.value("yaw", camera_yaw);
        camera_distance = camera.value("distance", camera_distance);
        if (camera.contains("position") && camera["position"].size() == 3) {
            camera_position = {
                camera["position"][0].get<float>(),
                camera["position"][1].get<float>(),
                camera["position"][2].get<float>()
            };
        }
        if (camera.contains("target") && camera["target"].size() == 3) {
            camera_target = {
                camera["target"][0].get<float>(),
                camera["target"][1].get<float>(),
                camera["target"][2].get<float>()
            };
        }
        // Camera mode distances
        camera_distance_locked = camera.value("distance_locked", camera_distance_locked);
        camera_distance_earth = camera.value("distance_earth", camera_distance_earth);
        camera_distance_moon = camera.value("distance_moon", camera_distance_moon);
        camera_distance_overview = camera.value("distance_overview", camera_distance_overview);
        camera_distance_solar_system = camera.value("distance_solar_system", camera_distance_solar_system);
        camera_distance_full_solar = camera.value("distance_full_solar", camera_distance_full_solar);
        camera_min_focus_distance = camera.value("min_focus_distance", camera_min_focus_distance);
    }
}