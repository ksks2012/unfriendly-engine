#ifndef BODY_H
#define BODY_H

#include "render_object.h"

#include <glm/glm.hpp>
#include <memory>

class Body {
public:    
    Body();
    Body(glm::vec3 pos, glm::vec3 vel, float m);

    glm::vec3 position;
    glm::vec3 velocity;
    float mass;
    std::unique_ptr<RenderObject> renderObject;
};

#endif