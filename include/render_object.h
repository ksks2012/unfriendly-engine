#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <vector>
#include <GL/glew.h>

class RenderObject {
public:
    RenderObject(const std::vector<GLfloat>&, const std::vector<GLuint>&);
    ~RenderObject();

    void render() const;
    GLuint getVao() const;
    GLuint getVbo() const;

    void renderTrajectory(size_t, size_t, size_t) const;

    // Update the data at the specified offset in the VBO
    void updateBuffer(GLintptr, GLsizei, const void*);

private:
    GLuint vao, vbo, ebo;
    GLsizei indexCount;
};

#endif