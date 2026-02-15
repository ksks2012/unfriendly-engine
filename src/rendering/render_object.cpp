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
    if (bridgeEBO_) {
        glDeleteBuffers(1, &bridgeEBO_);
    }
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

void RenderObject::ensureBridgeEBO() const {
    if (bridgeEBO_ == 0) {
        glGenBuffers(1, &bridgeEBO_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bridgeEBO_);
        // Allocate space for 2 indices; actual values written per draw call
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void RenderObject::renderTrajectory(size_t head, size_t count, size_t maxSize) const {
    if (count == 0) return;
    
    // Enable line smoothing for better visual quality
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(2.0f);
    glBindVertexArray(vao);
    
    if (count == maxSize && head > 0) {
        // Buffer is full (ring buffer wrapped around).
        // head points to the NEXT write position, so it's also the oldest point.
        //
        // Draw order: oldest data first → newest data
        //   Part 1: [head .. maxSize-1]
        //   Bridge:  maxSize-1 → 0
        //   Part 2: [0 .. head-1]

        // Part 1: old points [head .. maxSize-1]
        size_t firstPartCount = maxSize - head;
        glDrawArrays(GL_LINE_STRIP, head, firstPartCount);
        
        // Bridge: connect last vertex of part 1 (maxSize-1) to first vertex of part 2 (0)
        ensureBridgeEBO();
        GLuint bridgeIndices[2] = { static_cast<GLuint>(maxSize - 1), 0 };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bridgeEBO_);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(bridgeIndices), bridgeIndices);
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
        // Part 2: new points [0 .. head-1]
        glDrawArrays(GL_LINE_STRIP, 0, head);
    } else if (count == maxSize && head == 0) {
        // Special case: head wrapped exactly to 0, entire buffer is in order
        glDrawArrays(GL_LINE_STRIP, 0, maxSize);
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