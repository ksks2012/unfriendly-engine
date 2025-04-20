#include "rendering/camera.h"

Camera::Camera() : pitch(45.0f), yaw(45.0f), distance(500000.0f), mode(Mode::Free),
        smoothingFactor(0.1f), lockedOffset(0.0f, 0.0f, 100000.0f) {
    position = glm::vec3(0.0f, 0.0f, distance);
    target = glm::vec3(0.0f, 6371000.0f, 0.0f); // Initially pointing to Earth's surface
}

Camera::Camera(Config& config) : pitch(config.camera_pitch), yaw(config.camera_yaw), distance(config.camera_distance),
        mode(Mode::Free), smoothingFactor(0.1f), lockedOffset(0.0f, 0.0f, 100000.0f) {
    position = config.camera_position;
    target = config.camera_target;
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
            // Locked mode: automatically follow the rocket, maintaining relative offset
            target = rocketPosition;
            glm::vec3 desiredPosition = target + lockedOffset;
            // Smooth interpolation
            smoothedPosition = glm::mix(smoothedPosition, desiredPosition, smoothingFactor);
            smoothedTarget = target;
            position = smoothedPosition;
            break;
        }
        case Mode::Fixed: {
            // Fixed mode: keep pointing at the fixed target, no need to update the target
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
    distance = glm::clamp(distance, 1000.0f, 1000000.0f);
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

void Camera::setFixedTarget(const glm::vec3& fixedTarget) {
    target = fixedTarget;
    smoothedTarget = target;
}