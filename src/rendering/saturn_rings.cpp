#include "rendering/saturn_rings.h"

#include <cmath>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

SaturnRings::SaturnRings(float saturnRadius) : saturnRadius_(saturnRadius) {}

SaturnRings::~SaturnRings() {
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (ebo_) glDeleteBuffers(1, &ebo_);
    if (texture_) glDeleteTextures(1, &texture_);
    if (shaderProgram_) glDeleteProgram(shaderProgram_);
}

void SaturnRings::init() {
    // Generate mesh
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    generateDiskMesh(vertices, indices);
    
    // Create VAO, VBO, EBO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    
    glBindVertexArray(vao_);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
                 vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);
    
    // Position attribute (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute (2 floats: radial position and angle)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 
                         (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    indexCount_ = static_cast<GLsizei>(indices.size());
    
    // Generate texture
    generateRingTexture();
    
    // Create shader
    createShader();
}

void SaturnRings::generateDiskMesh(std::vector<float>& vertices, 
                                    std::vector<unsigned int>& indices) {
    const int radialSegments = 64;   // Segments around the ring
    const int ringSegments = 32;     // Segments from inner to outer radius
    
    float innerRadius = saturnRadius_ * INNER_RADIUS_RATIO;
    float outerRadius = saturnRadius_ * OUTER_RADIUS_RATIO;
    
    // Generate vertices
    for (int r = 0; r <= ringSegments; ++r) {
        float t = static_cast<float>(r) / ringSegments;
        float radius = innerRadius + t * (outerRadius - innerRadius);
        float radiusRatio = INNER_RADIUS_RATIO + t * (OUTER_RADIUS_RATIO - INNER_RADIUS_RATIO);
        
        for (int a = 0; a <= radialSegments; ++a) {
            float angle = static_cast<float>(a) / radialSegments * 2.0f * M_PI;
            
            // Position (flat disk in XZ plane, Y = 0)
            float x = radius * std::cos(angle);
            float y = 0.0f;
            float z = radius * std::sin(angle);
            
            // Texture coordinates: 
            // u = radial position (0 = inner, 1 = outer) - for ring texture lookup
            // v = angle (for potential future use)
            float u = t;
            float v = static_cast<float>(a) / radialSegments;
            
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }
    
    // Generate indices for triangle strip
    for (int r = 0; r < ringSegments; ++r) {
        for (int a = 0; a < radialSegments; ++a) {
            int current = r * (radialSegments + 1) + a;
            int next = current + radialSegments + 1;
            
            // Two triangles per quad
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);
            
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }
}

float SaturnRings::getRingOpacity(float radiusRatio) {
    // Saturn's ring structure (approximate, based on real data):
    // D Ring: 1.11 - 1.24 Saturn radii (very faint)
    // C Ring: 1.24 - 1.53 Saturn radii (dim)
    // B Ring: 1.53 - 1.95 Saturn radii (brightest, most opaque)
    // Cassini Division: 1.95 - 2.02 Saturn radii (gap)
    // A Ring: 2.02 - 2.27 Saturn radii (bright)
    //   Encke Gap: ~2.21 Saturn radii (narrow gap)
    // F Ring: ~2.32 Saturn radii (thin, faint)
    
    // D Ring (very faint)
    if (radiusRatio < 1.24f) {
        return 0.1f;
    }
    // C Ring (dim)
    else if (radiusRatio < 1.53f) {
        return 0.3f;
    }
    // B Ring (bright, opaque)
    else if (radiusRatio < 1.95f) {
        // Slightly varying opacity within B Ring
        float t = (radiusRatio - 1.53f) / (1.95f - 1.53f);
        return 0.7f + 0.2f * std::sin(t * M_PI);
    }
    // Cassini Division (gap)
    else if (radiusRatio < 2.02f) {
        return 0.05f;
    }
    // A Ring
    else if (radiusRatio < 2.27f) {
        // Check for Encke Gap (~2.21)
        if (radiusRatio > 2.20f && radiusRatio < 2.22f) {
            return 0.1f;  // Encke Gap
        }
        return 0.6f;
    }
    // Beyond main rings (F Ring area, very faint)
    else {
        return 0.05f;
    }
}

glm::vec3 SaturnRings::getRingColor(float radiusRatio) {
    // Ring colors vary slightly:
    // - Inner rings (C, B) are slightly more tan/brown
    // - Outer rings (A) are more gray/white
    // - Overall color is grayish-white with subtle tan tint
    
    if (radiusRatio < 1.53f) {
        // C Ring and D Ring: slightly brownish
        return glm::vec3(0.75f, 0.70f, 0.62f);
    }
    else if (radiusRatio < 1.95f) {
        // B Ring: cream/tan color
        return glm::vec3(0.85f, 0.80f, 0.70f);
    }
    else if (radiusRatio < 2.02f) {
        // Cassini Division: dark
        return glm::vec3(0.2f, 0.2f, 0.2f);
    }
    else {
        // A Ring: more gray/white
        return glm::vec3(0.80f, 0.78f, 0.75f);
    }
}

void SaturnRings::generateRingTexture() {
    const int textureWidth = 256;
    std::vector<unsigned char> textureData(textureWidth * 4);  // RGBA
    
    for (int i = 0; i < textureWidth; ++i) {
        float t = static_cast<float>(i) / (textureWidth - 1);
        float radiusRatio = INNER_RADIUS_RATIO + t * (OUTER_RADIUS_RATIO - INNER_RADIUS_RATIO);
        
        glm::vec3 color = getRingColor(radiusRatio);
        float opacity = getRingOpacity(radiusRatio);
        
        textureData[i * 4 + 0] = static_cast<unsigned char>(color.r * 255);
        textureData[i * 4 + 1] = static_cast<unsigned char>(color.g * 255);
        textureData[i * 4 + 2] = static_cast<unsigned char>(color.b * 255);
        textureData[i * 4 + 3] = static_cast<unsigned char>(opacity * 255);
    }
    
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_1D, texture_);
    
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, textureWidth, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, textureData.data());
    
    glBindTexture(GL_TEXTURE_1D, 0);
}

void SaturnRings::createShader() {
    // Vertex shader for rings
    const char* vertexSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec2 aTexCoord;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        out vec2 TexCoord;
        
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    // Fragment shader for rings with alpha blending
    const char* fragmentSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        
        uniform sampler1D ringTexture;
        
        void main() {
            // Sample ring texture based on radial position (TexCoord.x)
            vec4 ringColor = texture(ringTexture, TexCoord.x);
            
            // Discard nearly transparent fragments for better performance
            if (ringColor.a < 0.01) {
                discard;
            }
            
            FragColor = ringColor;
        }
    )";
    
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);
    
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Saturn rings vertex shader compilation failed: " << infoLog << std::endl;
    }
    
    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Saturn rings fragment shader compilation failed: " << infoLog << std::endl;
    }
    
    // Link program
    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vertexShader);
    glAttachShader(shaderProgram_, fragmentShader);
    glLinkProgram(shaderProgram_);
    
    glGetProgramiv(shaderProgram_, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram_, 512, nullptr, infoLog);
        std::cerr << "Saturn rings shader linking failed: " << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Get uniform locations
    modelLoc_ = glGetUniformLocation(shaderProgram_, "model");
    viewLoc_ = glGetUniformLocation(shaderProgram_, "view");
    projLoc_ = glGetUniformLocation(shaderProgram_, "projection");
    textureLoc_ = glGetUniformLocation(shaderProgram_, "ringTexture");
}

void SaturnRings::render(const glm::mat4& model, const glm::mat4& view,
                          const glm::mat4& projection, float scale) const {
    if (!shaderProgram_ || !vao_ || !texture_) return;
    
    // Save current OpenGL state
    GLboolean depthMask;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    GLboolean cullFace;
    glGetBooleanv(GL_CULL_FACE, &cullFace);
    GLboolean blend;
    glGetBooleanv(GL_BLEND, &blend);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable depth writing (but keep depth testing) for transparent objects
    glDepthMask(GL_FALSE);
    
    // Use ring shader
    glUseProgram(shaderProgram_);
    
    // Create scaled model matrix
    // The ring mesh is already sized in meters, so we just need to apply scale
    glm::mat4 scaledModel = glm::scale(model, glm::vec3(scale, scale, scale));
    
    // Apply Saturn's axial tilt (26.73 degrees)
    // Rotate around Z axis to tilt the ring plane
    float axialTilt = glm::radians(26.73f);
    scaledModel = glm::rotate(scaledModel, axialTilt, glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Set uniforms
    glUniformMatrix4fv(modelLoc_, 1, GL_FALSE, glm::value_ptr(scaledModel));
    glUniformMatrix4fv(viewLoc_, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc_, 1, GL_FALSE, glm::value_ptr(projection));
    
    // Bind ring texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, texture_);
    glUniform1i(textureLoc_, 0);
    
    // Disable face culling to render both sides of the ring
    glDisable(GL_CULL_FACE);
    
    // Draw the ring
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    
    // Restore OpenGL state
    glDepthMask(depthMask);
    if (cullFace) glEnable(GL_CULL_FACE);
    else glDisable(GL_CULL_FACE);
    if (!blend) glDisable(GL_BLEND);
    
    glBindTexture(GL_TEXTURE_1D, 0);
}
