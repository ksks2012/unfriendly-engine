#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <vector>

#include <GL/glew.h>

class RenderObject {
public:
    RenderObject(const std::vector<GLfloat>&, const std::vector<GLuint>&);
    ~RenderObject();

    void render() const;

private:
    GLuint vao, vbo, ebo;
    GLsizei indexCount;
};

#endif