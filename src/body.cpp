#include "body.h"

Body::Body() : position(0.0f), velocity(0.0f), mass(1.0f) {
};

Body::Body(const Body& other) : position(other.position), velocity(other.velocity), mass(other.mass) {
}

Body::Body(glm::vec3 pos, glm::vec3 vel, float m) : position(pos), velocity(vel), mass(m) {
}

Body& Body::operator=(const Body& other) {
    if (this != &other) {
        position = other.position;
        velocity = other.velocity;
        mass = other.mass;
    }
    return *this;
}
