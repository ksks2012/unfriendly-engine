#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <glm/glm.hpp>

// Log levels
enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

// Abstract logger interface
class ILogger {
public:
    virtual ~ILogger() = default;
    
    // General logging method
    virtual void log(LogLevel level, const std::string& module, const std::string& message) = 0;
    
    // Structured logging method (orbital data)
    virtual void log_orbit(LogLevel level, const std::string& module, float time,
                          const glm::vec3& position, float radius, const glm::vec3& velocity) = 0;
    
    // Set log level
    virtual void set_level(LogLevel level) = 0;
};

// Logging macros (simplify calls)
#define LOG_DEBUG(logger, module, msg) (logger)->log(LogLevel::DEBUG, module, msg)
#define LOG_INFO(logger, module, msg) (logger)->log(LogLevel::INFO, module, msg)
#define LOG_ERROR(logger, module, msg) (logger)->log(LogLevel::ERROR, module, msg)
#define LOG_WARN(logger, module, msg) (logger)->log(LogLevel::WARN, module, msg)
#define LOG_ORBIT(logger, module, time, pos, radius, vel) \
    (logger)->log_orbit(LogLevel::DEBUG, module, time, pos, radius, vel)

#endif