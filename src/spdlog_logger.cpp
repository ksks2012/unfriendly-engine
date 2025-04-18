#include "spdlog_logger.h"

SpdlogLogger::SpdlogLogger() {
    // Initialize the main logger (console and log file)
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%n] [%l] %v");
    
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/simulation.log", 1024 * 1024 * 5, 3); // 5MB, 3 files
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%n] [%l] %v");
    
    std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};
    logger_ = std::make_shared<spdlog::logger>("simulation", sinks.begin(), sinks.end());
    logger_->set_level(spdlog::level::debug);
    logger_->flush_on(spdlog::level::info);
    spdlog::register_logger(logger_);
    
    // Initialize the CSV logger (dedicated to structured data)
    auto csv_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/orbit.csv", 1024 * 1024 * 5, 3);
    csv_sink->set_pattern("%v"); // No formatting, direct message output
    csv_logger_ = std::make_shared<spdlog::logger>("csv", csv_sink);
    csv_logger_->set_level(spdlog::level::info);
    csv_logger_->flush_on(spdlog::level::info);
    spdlog::register_logger(csv_logger_);
}

void SpdlogLogger::log(LogLevel level, const std::string& module, const std::string& message) {
    switch (level) {
        case LogLevel::DEBUG:
            logger_->debug("[{}] {}", module, message);
            break;
        case LogLevel::INFO:
            logger_->info("[{}] {}", module, message);
            break;
        case LogLevel::WARN:
            logger_->warn("[{}] {}", module, message);
            break;
        case LogLevel::ERROR:
            logger_->error("[{}] {}", module, message);
            break;
    }
}

void SpdlogLogger::log_orbit(LogLevel level, const std::string& module, float time,
               const glm::vec3& position, float radius, const glm::vec3& velocity) {
    // CSV format: module,time,pos_x,pos_y,pos_z,radius,vel_x,vel_y,vel_z
    std::string csv_message = fmt::format("{},{},{},{},{},{},{},{},{}",
        module, time, position.x, position.y, position.z, radius,
        velocity.x, velocity.y, velocity.z);
    csv_logger_->info("{}", csv_message); // Write to CSV logger
    
    // Console and log file: human-readable format
    std::string message = fmt::format("Pos=({}, {}, {}), Radius={:.2f}, Vel=({}, {}, {})",
        position.x, position.y, position.z, radius,
        velocity.x, velocity.y, velocity.z);
    log(level, module, message);
}

void SpdlogLogger::set_level(LogLevel level) {
    spdlog::level::level_enum spd_level;
    switch (level) {
        case LogLevel::DEBUG: spd_level = spdlog::level::debug; break;
        case LogLevel::INFO: spd_level = spdlog::level::info; break;
        case LogLevel::WARN: spd_level = spdlog::level::warn; break;
        case LogLevel::ERROR: spd_level = spdlog::level::err; break;
    }
    logger_->set_level(spd_level);
    csv_logger_->set_level(spd_level);
}