#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

class Shader {
public:
    Shader();
    void init();
    ~Shader();

    void use() const;
    void setMat4(const std::string&, const glm::mat4&) const;
    void setVec4(const std::string&, const glm::vec4&) const;

private:
    GLuint program, vertexShader, fragmentShader;
    GLuint compileShader(GLenum, const char*);
};

#endif