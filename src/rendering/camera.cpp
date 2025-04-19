#include "rendering/camera.h"

Camera::Camera() : pitch(45.0f), yaw(45.0f), distance(500000.0f) {
    position = glm::vec3(0.0f, 0.0f, distance);
    target = glm::vec3(0.0f, 6371000.0f, 0.0f); // Initially pointing to Earth's surface
}

// Update camera position
void Camera::update(const glm::vec3& rocketPosition) {
    target = rocketPosition;
    // Calculate camera position based on pitch and yaw angles
    float radPitch = glm::radians(pitch);
    float radYaw = glm::radians(yaw);
    position.x = target.x + distance * cos(radPitch) * sin(radYaw);
    position.y = target.y + distance * sin(radPitch);
    position.z = target.z + distance * cos(radPitch) * cos(radYaw);
}

// Get view matrix
glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
}

// Control camera rotation
void Camera::rotate(float deltaPitch, float deltaYaw) {
    pitch += deltaPitch;
    yaw += deltaYaw;
    // Clamp pitch angle to avoid flipping
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
}

// Control zoom
void Camera::zoom(float deltaDistance) {
    distance += deltaDistance;
    distance = glm::clamp(distance, 1000.0f, 1000000.0f); // Clamp distance range
}