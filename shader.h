#ifndef SHADER_H
#define SHADER_H

#include <iostream>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    Shader() {
        const char* vertexSource = R"(
            #version 330 core
            layout(location = 0) in vec3 aPos;
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            void main() {
                gl_Position = projection * view * model * vec4(aPos, 1.0);
            }
        )";

        const char* fragmentSource = R"(
            #version 330 core
            out vec4 FragColor;
            uniform vec4 color;
            void main() {
                FragColor = color;
            }
        )";

        vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
        fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Shader linking failed: " << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    ~Shader() {
        glDeleteProgram(program);
    }

    void use() const {
        glUseProgram(program);
    }

    void setMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void setVec4(const std::string& name, const glm::vec4& vec) const {
        glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(vec));
    }

private:
    GLuint program, vertexShader, fragmentShader;

    GLuint compileShader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        }
        return shader;
    }
};

#endif