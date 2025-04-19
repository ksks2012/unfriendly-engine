#ifndef SPDLOG_LOGGER_H
#define SPDLOG_LOGGER_H

#include "logging/logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>

class SpdlogLogger : public ILogger {
public:
    SpdlogLogger();
    ~SpdlogLogger() override = default;

    void log(LogLevel level, const std::string& module, const std::string& message) override;
    void log_orbit(LogLevel level, const std::string& module, float time,
                   const glm::vec3& position, float radius, const glm::vec3& velocity) override;

    void set_level(LogLevel level) override;

private:
    std::shared_ptr<spdlog::logger> logger_; // Main logger (console and log file)
    std::shared_ptr<spdlog::logger> csv_logger_; // CSV logger (structured data)
};

#endif