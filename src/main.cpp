#include <iostream>

#include "app.h"
#include "config.h"

int main() {
    try {
        Config config;
        config.loadFromFile("etc/config.json");
        App app("Rocket Simulation", 800, 600, config);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}