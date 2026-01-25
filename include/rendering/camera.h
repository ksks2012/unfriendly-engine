#ifndef CAMERA_H
#define CAMERA_H

#include "app/config.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    enum class Mode {
        Free,         // Free view, manually controlled
        Locked,       // Locked on the rocket, automatically follows
        FixedEarth,   // Fixed on Earth center
        FixedMoon,    // Fixed on Moon center
        Overview,     // Overview of Earth-Moon system
        SolarSystem   // Solar system view (centered on Sun)
    };

    glm::vec3 position;    // Camera position
    glm::vec3 target;      // Target position (rocket)
    float pitch;           // Pitch angle (degrees)
    float yaw;             // Yaw angle (degrees)
    float distance;        // Distance to the target
    Mode mode;            // Camera mode
    glm::vec3 fixedTarget; // Fixed target position (for Fixed mode)

public:
    Camera();
    Camera(Config& config);
    // TODO: constuctor with config parameters

    void update(const glm::vec3& rocketPosition);
    glm::mat4 getViewMatrix() const;
    void rotate(float deltaPitch, float deltaYaw);
    void zoom(float deltaDistance);

    void setMode(Mode newMode);
    void setFixedTarget(const glm::vec3& fixedTarget);
    const char* getModeName() const; // Get current mode name for display

private:
    glm::vec3 smoothedPosition; // Smoothed camera position
    glm::vec3 smoothedTarget;   // Smoothed target position
    float smoothingFactor;      // Smoothing factor (0.0 to 1.0)
    glm::vec3 lockedOffset;     // Relative offset in Locked mode
};

#endif // CAMERA_H