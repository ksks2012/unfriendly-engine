#include "rendering/camera.h"

Camera::Camera() : pitch(45.0f), yaw(45.0f), distance(500000.0f), mode(Mode::Free),
        smoothingFactor(0.1f), lockedOffset(0.5f, 0.7f, 0.3f), earthPosition(0.0f) {
    position = glm::vec3(0.0f, 0.0f, distance);
    target = glm::vec3(0.0f, 6371000.0f, 0.0f); // Initially pointing to Earth's surface
    fixedTarget = glm::vec3(0.0f, 0.0f, 0.0f);
}

Camera::Camera(Config& config) : pitch(config.camera_pitch), yaw(config.camera_yaw), distance(config.camera_distance),
        mode(Mode::Free), smoothingFactor(0.1f), lockedOffset(0.5f, 0.7f, 0.3f), earthPosition(0.0f) {
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
            // In heliocentric coordinates, we need the direction relative to Earth, not the Sun
            glm::vec3 relativeToEarth = rocketPosition - earthPosition;
            float distFromEarth = glm::length(relativeToEarth);
            
            // Earth radius in rendering units (km)
            const float earthRadiusKm = 6371.0f;
            // Safety margin outside Earth surface
            const float safetyMargin = earthRadiusKm * 0.1f; // 10% of Earth radius (~637 km)
            
            glm::vec3 radialDir;
            if (distFromEarth < 0.001f) {
                // Fallback if rocket is at Earth center
                radialDir = glm::vec3(0.0f, 1.0f, 0.0f);
            } else {
                radialDir = relativeToEarth / distFromEarth;
            }
            
            // Create a tangent direction (perpendicular to radial)
            // Use Z-axis as reference, but handle edge cases
            glm::vec3 refUp = glm::vec3(0.0f, 0.0f, 1.0f);
            if (std::abs(glm::dot(radialDir, refUp)) > 0.99f) {
                refUp = glm::vec3(1.0f, 0.0f, 0.0f);
            }
            glm::vec3 tangentDir = glm::normalize(glm::cross(radialDir, refUp));
            glm::vec3 binormalDir = glm::normalize(glm::cross(tangentDir, radialDir));
            
            // Apply lockedOffset for user-controlled rotation
            // The offset is in local coordinates (radial, tangent, binormal)
            // For initial view: place camera above and slightly behind rocket
            // radialDir (y=0.7): above the rocket (away from Earth)
            // tangentDir (x=0.5): slightly to the side
            // binormalDir (z=0.3): slightly behind
            glm::vec3 offsetDir = glm::normalize(
                radialDir * lockedOffset.y +      // Up/down (radial) - dominant
                tangentDir * lockedOffset.x +     // Left/right
                binormalDir * lockedOffset.z      // Forward/back
            );
            
            // Calculate rocket's altitude above Earth surface
            float rocketAltitude = std::max(0.0f, distFromEarth - earthRadiusKm);
            
            // Calculate effective camera distance for locked mode
            // Use a reasonable distance based on altitude
            float effectiveDistance;
            if (rocketAltitude < 10.0f) {
                // On or very near surface: use fixed reasonable distance
                effectiveDistance = 100.0f;  // 100 km - can see rocket and Earth
            } else if (rocketAltitude < 1000.0f) {
                // Low altitude: scale with altitude
                effectiveDistance = std::max(100.0f, rocketAltitude * 2.0f);
            } else if (rocketAltitude < 10000.0f) {
                // Medium altitude
                effectiveDistance = rocketAltitude * 0.5f;
            } else {
                // High altitude: use user distance but cap it
                effectiveDistance = std::min(distance, rocketAltitude * 2.0f);
            }
            
            // Ensure minimum distance for visibility
            effectiveDistance = std::max(effectiveDistance, 20.0f);
            
            // Calculate proposed camera position
            glm::vec3 proposedPosition = target + offsetDir * effectiveDistance;
            
            // Check if camera would be inside Earth
            glm::vec3 cameraRelativeToEarth = proposedPosition - earthPosition;
            float cameraDistFromEarthCenter = glm::length(cameraRelativeToEarth);
            
            if (cameraDistFromEarthCenter < earthRadiusKm + safetyMargin) {
                // Camera would be inside Earth - reposition it
                // Place camera on the same side of Earth as the rocket (radially outward)
                // Use radial direction to push camera outside Earth
                glm::vec3 cameraRadialDir = radialDir; // Same direction as rocket from Earth
                
                // Position camera outside Earth, looking at rocket
                float minCameraDistFromEarth = earthRadiusKm + safetyMargin;
                proposedPosition = earthPosition + cameraRadialDir * (minCameraDistFromEarth + effectiveDistance * 0.5f);
            }
            
            // Final check: ensure camera is outside Earth and can see rocket
            // The line from camera to rocket should not pass through Earth
            // Simple check: camera should be on the same side of Earth as rocket
            glm::vec3 finalCameraRelToEarth = proposedPosition - earthPosition;
            float dotWithRocket = glm::dot(glm::normalize(finalCameraRelToEarth), radialDir);
            
            if (dotWithRocket < 0.0f && glm::length(finalCameraRelToEarth) < earthRadiusKm * 2.0f) {
                // Camera is on opposite side of Earth and close - line of sight blocked
                // Move camera to rocket's side
                proposedPosition = earthPosition + radialDir * (distFromEarth + effectiveDistance);
            }
            
            // NO SMOOTHING - camera follows rocket instantly
            // This is critical for high-speed flight
            position = proposedPosition;
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
            // Solar system view: centered on Sun, view the inner solar system
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
        case Mode::FullSolarSystem: {
            // Full solar system view: see all 8 planets including Neptune (~30 AU)
            target = fixedTarget; // Sun center
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
    // Note: In Locked mode, distance is used directly in the update() method
    // lockedOffset only controls the direction, not the distance
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

void Camera::setEarthPosition(const glm::vec3& earthPos) {
    earthPosition = earthPos;
}

const char* Camera::getModeName() const {
    switch (mode) {
        case Mode::Free: return "Free View";
        case Mode::Locked: return "Locked on Rocket";
        case Mode::FixedEarth: return "Earth View";
        case Mode::FixedMoon: return "Moon View";
        case Mode::Overview: return "Earth-Moon Overview";
        case Mode::SolarSystem: return "Inner Solar System";
        case Mode::FullSolarSystem: return "Full Solar System (8 Planets)";
        default: return "Unknown";
    }
}