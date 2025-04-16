#include "body.h"

Body::Body() : position(0.0f), velocity(0.0f), mass(1.0f) {
};

Body::Body(glm::vec3 pos, glm::vec3 vel, float m) : position(pos), velocity(vel), mass(m) {
}
