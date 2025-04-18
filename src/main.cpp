#include <iostream>

#include "app.h"
#include "config.h"
#include "spdlog_logger.h"


int main() {
    try {
        Config config;
        config.loadFromFile("etc/config.json");
        auto logger = std::make_shared<SpdlogLogger>();
        App app("Rocket Simulation", 800, 600, config, logger);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}