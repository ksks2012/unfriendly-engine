#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 position;    // Camera position
    glm::vec3 target;      // Target position (rocket)
    float pitch;           // Pitch angle (degrees)
    float yaw;             // Yaw angle (degrees)
    float distance;        // Distance to the target

public:
    Camera();

    void update(const glm::vec3& rocketPosition);
    glm::mat4 getViewMatrix() const;
    void rotate(float deltaPitch, float deltaYaw);
    void zoom(float deltaDistance);
};