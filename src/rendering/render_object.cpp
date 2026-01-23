#include "rendering/render_object.h"

#include <iostream>

RenderObject::RenderObject(const std::vector<GLfloat>& vertices, const std::vector<GLuint>& indices) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Use GL_DYNAMIC_DRAW for trajectory data that updates frequently
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_DYNAMIC_DRAW);

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
    if (count == 0) return;
    
    // Enable line smoothing for better visual quality
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(2.0f);
    glBindVertexArray(vao);
    
    if (count == maxSize) {
        // Buffer is full, need to draw in two parts
        // First part: from head to end of buffer (older points)
        size_t firstPartCount = maxSize - head;
        if (firstPartCount > 0) {
            glDrawArrays(GL_LINE_STRIP, head, firstPartCount);
        }
        // Second part: from beginning to head (newer points)
        if (head > 0) {
            glDrawArrays(GL_LINE_STRIP, 0, head);
        }
        // Connect the two parts with a single line segment
        // This requires drawing the last point of first part to first point of second part
        if (firstPartCount > 0 && head > 0) {
            // Draw connection line between end and start
            GLint connectIndices[2] = { static_cast<GLint>(maxSize - 1), 0 };
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, connectIndices);
        }
    } else {
        // Buffer not full, draw from beginning to count
        glDrawArrays(GL_LINE_STRIP, 0, count);
    }
    
    glLineWidth(1.0f);
    glDisable(GL_LINE_SMOOTH);
    glBindVertexArray(0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error in RenderObject::renderTrajectory: " << err 
                  << ", head: " << head << ", count: " << count << std::endl;
    }
}

void RenderObject::renderOrbit(size_t count) const {
    if (count == 0) return;
    
    // Enable line smoothing for better visual quality
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(1.5f);
    glBindVertexArray(vao);
    
    // Use GL_LINE_LOOP for closed orbits (automatically connects last point to first)
    glDrawArrays(GL_LINE_LOOP, 0, count);
    
    glLineWidth(1.0f);
    glDisable(GL_LINE_SMOOTH);
    glBindVertexArray(0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error in RenderObject::renderOrbit: " << err 
                  << ", count: " << count << std::endl;
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