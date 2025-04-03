#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <vector>

// Shader sources
const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 color;
    void main() {
        FragColor = color;
    }
)";

// Compile shader
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

// Link program
GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

// Geometry structure
struct RenderObject {
    GLuint vao, vbo, ebo;
    GLsizei indexCount;

    RenderObject(const std::vector<GLfloat>& vertices, const std::vector<GLuint>& indices) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
        indexCount = static_cast<GLsizei>(indices.size());
    }

    ~RenderObject() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }
};

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Rocket Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Shader program
    GLuint shaderProgram = createShaderProgram();

    // Rocket geometry
    std::vector<GLfloat> rocketVertices = {
        0.0f, 0.0f, 0.0f,  // Bottom
        0.0f, 10.0f, 0.0f, // Top
        2.0f, 0.0f, 0.0f   // Right
    };
    std::vector<GLuint> rocketIndices = {0, 1, 2};
    RenderObject rocket(rocketVertices, rocketIndices);

    // Ground geometry
    std::vector<GLfloat> groundVertices = {
        -500.0f, 0.0f, -500.0f,
        500.0f, 0.0f, -500.0f,
        500.0f, 0.0f, 500.0f,
        -500.0f, 0.0f, 500.0f
    };
    std::vector<GLuint> groundIndices = {0, 1, 2, 0, 2, 3};
    RenderObject ground(groundVertices, groundIndices);

    // Simulation state
    glm::vec3 rocketPos(0.0f, 6371.0f, 0.0f);
    float cameraDistance = 2000.0f;
    float timeScale = 1.0f;

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Input handling
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            timeScale += 0.1f;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            timeScale = std::max(timeScale - 0.1f, 0.1f);
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cameraDistance = std::max(cameraDistance - 100.0f, 500.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cameraDistance = std::min(cameraDistance + 100.0f, 5000.0f);
        }

        // Clear screen
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Scene rendering
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float sceneHeight = height * 0.8f;
        glViewport(0, 0, width, static_cast<int>(sceneHeight));

        glUseProgram(shaderProgram);

        glm::vec3 scaledPos = rocketPos / 1000.0f;
        glm::vec3 cameraPos(0.0f, 6371.0f + cameraDistance, cameraDistance);
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 6371.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / sceneHeight, 0.1f, cameraDistance * 2.0f);

        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint colorLoc = glGetUniformLocation(shaderProgram, "color");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Render ground
        glm::mat4 groundModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 6371.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(groundModel));
        glUniform4f(colorLoc, 0.0f, 0.8f, 0.0f, 1.0f);
        glBindVertexArray(ground.vao);
        glDrawElements(GL_TRIANGLES, ground.indexCount, GL_UNSIGNED_INT, nullptr);

        // Render rocket
        glm::mat4 rocketModel = glm::translate(glm::mat4(1.0f), scaledPos);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(rocketModel));
        glUniform4f(colorLoc, 0.8f, 0.8f, 0.8f, 1.0f);
        glBindVertexArray(rocket.vao);
        glDrawElements(GL_TRIANGLES, rocket.indexCount, GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);

        // UI rendering with ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(10, sceneHeight + 10));
        ImGui::SetNextWindowSize(ImVec2(width - 20, height * 0.2f - 20));
        ImGui::Begin("Simulation Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("Time Scale: %.1f", timeScale);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}