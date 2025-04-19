#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <vector>
#include <GL/glew.h>

class IRenderObject {
public:
    // IRenderObject() = default;
    virtual ~IRenderObject() = default;
    virtual void render() const = 0;
    virtual GLuint getVao() const = 0;
    virtual GLuint getVbo() const = 0;
    virtual void renderTrajectory(size_t, size_t, size_t) const = 0;
    virtual void updateBuffer(GLintptr, GLsizei, const void*) = 0;
};
    
class RenderObject : public IRenderObject{
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