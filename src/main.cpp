#include <iostream>

#include "app.h"

int main() {
    try {
        App app("Rocket Simulation", 800, 600);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}