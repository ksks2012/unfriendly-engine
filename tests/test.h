#ifndef TEST_H
#define TEST_H

#include "rendering/render_object.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>


class MockRenderObject : public IRenderObject {
public:
    MOCK_METHOD(void, render, (), (const, override));
    MOCK_METHOD(GLuint, getVao, (), (const, override));
    MOCK_METHOD(GLuint, getVbo, (), (const, override));
    MOCK_METHOD(void, renderTrajectory, (size_t, size_t, size_t), (const, override));
    MOCK_METHOD(void, updateBuffer, (GLintptr, GLsizei, const void*), (override));
};

class MockLogger : public ILogger {
public:
    MOCK_METHOD(void, log, (LogLevel level, const std::string& module, const std::string& message), (override));
    MOCK_METHOD(void, log_orbit, (LogLevel level, const std::string& module, float time, const glm::vec3& position, float radius, const glm::vec3& velocity), (override));
    MOCK_METHOD(void, set_level, (LogLevel level), (override));
};

#endif // TEST_H