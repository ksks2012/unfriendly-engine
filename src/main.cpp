#include <iostream>

#include "app/app.h"
#include "app/config.h"
#include "logging/spdlog_logger.h"


int main() {
    try {
        Config config;
        config.loadFromFile("etc/config.json");
        auto logger = std::make_shared<SpdlogLogger>();
        auto camera = Camera(config);
        App app("Rocket Simulation", 800, 600, config, logger, camera);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}