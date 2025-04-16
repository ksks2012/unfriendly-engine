#ifndef BODY_H
#define BODY_H

#include "render_object.h"

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>

class Body {
public:    
    Body();
    Body(const Body& other);
    Body(glm::vec3 pos, glm::vec3 vel, float m);

    Body& operator=(const Body& other);
    
    glm::vec3 position;
    glm::vec3 velocity;
    float mass;
    std::unique_ptr<IRenderObject> renderObject;
};

typedef std::unordered_map<std::string, std::unique_ptr<Body>> BODY_MAP;

#endif