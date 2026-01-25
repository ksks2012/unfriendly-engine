#include "rendering/camera.h"

Camera::Camera() : pitch(45.0f), yaw(45.0f), distance(500000.0f), mode(Mode::Free),
        smoothingFactor(0.1f), lockedOffset(0.0f, 200.0f, 200.0f) {
    position = glm::vec3(0.0f, 0.0f, distance);
    target = glm::vec3(0.0f, 6371000.0f, 0.0f); // Initially pointing to Earth's surface
    fixedTarget = glm::vec3(0.0f, 0.0f, 0.0f);
}

Camera::Camera(Config& config) : pitch(config.camera_pitch), yaw(config.camera_yaw), distance(config.camera_distance),
        mode(Mode::Free), smoothingFactor(0.1f), lockedOffset(0.0f, 200.0f, 200.0f) {
    position = config.camera_position;
    target = config.camera_target;
    fixedTarget = glm::vec3(0.0f, 0.0f, 0.0f);
}

// Update camera position
void Camera::update(const glm::vec3& rocketPosition) {
    switch (mode) {
        case Mode::Free: {
            // Free mode: manually controlled
            target = rocketPosition;
            float radPitch = glm::radians(pitch);
            float radYaw = glm::radians(yaw);
            position.x = target.x + distance * cos(radPitch) * sin(radYaw);
            position.y = target.y + distance * sin(radPitch);
            position.z = target.z + distance * cos(radPitch) * cos(radYaw);
            smoothedPosition = position;
            smoothedTarget = target;
            break;
        }
        case Mode::Locked: {
            // Locked mode: camera follows the rocket directly without smoothing lag
            // This ensures the rocket is always in view, even at high speeds
            
            // Target is always the current rocket position
            target = rocketPosition;
            
            // Calculate radial direction (from Earth center through rocket position)
            glm::vec3 radialDir = glm::normalize(rocketPosition);
            if (glm::length(rocketPosition) < 0.001f) {
                // Fallback if rocket is at origin
                radialDir = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            
            // Create a tangent direction (perpendicular to radial)
            glm::vec3 refUp = glm::vec3(0.0f, 0.0f, 1.0f);
            if (std::abs(glm::dot(radialDir, refUp)) > 0.99f) {
                refUp = glm::vec3(1.0f, 0.0f, 0.0f);
            }
            glm::vec3 tangentDir = glm::normalize(glm::cross(radialDir, refUp));
            
            // Position camera at an angle: mostly radial (up) with some tangent (side)
            // This shows the flat rocket triangle from an angle
            glm::vec3 offsetDir = glm::normalize(radialDir * 0.7f + tangentDir * 0.5f + glm::vec3(0.0f, 0.0f, 0.3f));
            
            // NO SMOOTHING - camera follows rocket instantly
            // This is critical for high-speed flight
            position = target + offsetDir * distance;
            smoothedPosition = position;
            smoothedTarget = target;
            break;
        }
        case Mode::FixedEarth: {
            // Fixed on Earth: camera looks at Earth center, can be manually rotated
            target = fixedTarget; // Earth center
            float radPitch = glm::radians(pitch);
            float radYaw = glm::radians(yaw);
            position.x = target.x + distance * cos(radPitch) * sin(radYaw);
            position.y = target.y + distance * sin(radPitch);
            position.z = target.z + distance * cos(radPitch) * cos(radYaw);
            smoothedPosition = position;
            smoothedTarget = target;
            break;
        }
        case Mode::FixedMoon: {
            // Fixed on Moon: camera looks at Moon center, can be manually rotated
            target = fixedTarget; // Moon center
            float radPitch = glm::radians(pitch);
            float radYaw = glm::radians(yaw);
            position.x = target.x + distance * cos(radPitch) * sin(radYaw);
            position.y = target.y + distance * sin(radPitch);
            position.z = target.z + distance * cos(radPitch) * cos(radYaw);
            smoothedPosition = position;
            smoothedTarget = target;
            break;
        }
        case Mode::Overview: {
            // Overview mode: view the entire Earth-Moon system
            // Target is the midpoint between Earth and Moon
            target = fixedTarget; // Should be set to midpoint in simulation
            float radPitch = glm::radians(pitch);
            float radYaw = glm::radians(yaw);
            position.x = target.x + distance * cos(radPitch) * sin(radYaw);
            position.y = target.y + distance * sin(radPitch);
            position.z = target.z + distance * cos(radPitch) * cos(radYaw);
            smoothedPosition = position;
            smoothedTarget = target;
            break;
        }
        case Mode::SolarSystem: {
            // Solar system view: centered on Sun, view the entire solar system
            target = fixedTarget; // Should be set to Sun center (origin in heliocentric coords)
            float radPitch = glm::radians(pitch);
            float radYaw = glm::radians(yaw);
            position.x = target.x + distance * cos(radPitch) * sin(radYaw);
            position.y = target.y + distance * sin(radPitch);
            position.z = target.z + distance * cos(radPitch) * cos(radYaw);
            smoothedPosition = position;
            smoothedTarget = target;
            break;
        }
    }
}

// Get view matrix
glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
}

// Control camera rotation
void Camera::rotate(float deltaPitch, float deltaYaw) {
    if (mode == Mode::Locked) {
        // Locked mode: fine-tune the offset angle
        float radPitch = glm::radians(deltaPitch);
        float radYaw = glm::radians(deltaYaw);
        // Rotate lockedOffset
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), radYaw, glm::vec3(0.0f, 1.0f, 0.0f)) *
                     glm::rotate(glm::mat4(1.0f), radPitch, glm::vec3(1.0f, 0.0f, 0.0f));
        lockedOffset = glm::mat3(rotation) * lockedOffset;
        } else {
        // Free or fixed mode: directly adjust pitch and yaw
        pitch += deltaPitch;
        yaw += deltaYaw;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
    }
}

// Control zoom
void Camera::zoom(float deltaDistance) {
    distance += deltaDistance;
    // Extended range to support different viewing modes
    distance = glm::clamp(distance, 1000.0f, 1000000000.0f); // Up to 1,000,000 km
    if (mode == Mode::Locked) {
        // Scale the length of lockedOffset
        lockedOffset = glm::normalize(lockedOffset) * distance;
    }
}

void Camera::setMode(Mode newMode) {
    mode = newMode;
    // Reset smoothing parameters to avoid jumps
    smoothedPosition = position;
    smoothedTarget = target;
}

void Camera::setFixedTarget(const glm::vec3& newTarget) {
    fixedTarget = newTarget;
    target = newTarget;
    smoothedTarget = newTarget;
}

const char* Camera::getModeName() const {
    switch (mode) {
        case Mode::Free: return "Free View";
        case Mode::Locked: return "Locked on Rocket";
        case Mode::FixedEarth: return "Earth View";
        case Mode::FixedMoon: return "Moon View";
        case Mode::Overview: return "System Overview";
        case Mode::SolarSystem: return "Solar System View";
        default: return "Unknown";
    }
}