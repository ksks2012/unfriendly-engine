#include "render_object.h"

#include <iostream>

RenderObject::RenderObject(const std::vector<GLfloat>& vertices, const std::vector<GLuint>& indices) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    if (!indices.empty()) {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
        indexCount = static_cast<GLsizei>(indices.size());
    } else {
         // Use vertex count when no indices
        indexCount = static_cast<GLsizei>(vertices.size() / 3);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error in RenderObject::RenderObject: " << err << std::endl;
    }
}

RenderObject::~RenderObject() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void RenderObject::render() const {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

GLuint RenderObject::getVao() const {
    return vao;
}

GLuint RenderObject::getVbo() const {
    return vbo;
}

void RenderObject::renderTrajectory(size_t head, size_t count, size_t maxSize) const {
    glLineWidth(5.0f);
    glBindVertexArray(vao);
    if (count == maxSize) {
        glDrawArrays(GL_LINE_STRIP, head, maxSize - head);
        glDrawArrays(GL_LINE_STRIP, 0, head);
    } else {
        glDrawArrays(GL_LINE_STRIP, 0, count);
    }
    glLineWidth(1.0f);
    glBindVertexArray(0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error in RenderObject::renderTrajectory: " << err 
                  << ", head: " << head << ", count: " << count << std::endl;
    }
}

void RenderObject::updateBuffer(GLintptr offset, GLsizei size, const void* data) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error in RenderObject::updateBuffer: " << err 
                  << ", offset: " << offset << ", size: " << size << std::endl;
    }
}