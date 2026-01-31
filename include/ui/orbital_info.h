#ifndef ORBITAL_INFO_H
#define ORBITAL_INFO_H

#include "core/orbital_elements.h"
#include "core/rocket.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <memory>

// Forward declaration
class Body;
using BODY_MAP = std::unordered_map<std::string, std::unique_ptr<Body>>;

/**
 * OrbitalInfo - UI panel displaying orbital elements
 */
class OrbitalInfo {
public:
    OrbitalInfo() = default;
    
    /**
     * Render the orbital info panel
     * @param rocket The rocket to display orbital info for
     * @param bodies Map of celestial bodies
     * @param panelX X position of the panel
     * @param panelY Y position of the panel
     */
    void render(const Rocket& rocket, const BODY_MAP& bodies, 
                float panelX, float panelY);

private:
    // Find the most influential body (for determining SOI)
    std::string findDominantBody(const glm::dvec3& position, const BODY_MAP& bodies) const;
    
    // Render a single orbital element with label and value
    void renderElement(const char* label, const std::string& value, 
                       ImU32 color = IM_COL32(255, 255, 255, 255)) const;
    
    // Render a progress bar for values like eccentricity
    void renderProgressBar(const char* label, float value, float maxValue,
                           const char* overlay, ImU32 color) const;
    
    // Colors
    static constexpr ImU32 COLOR_LABEL = IM_COL32(180, 180, 180, 255);
    static constexpr ImU32 COLOR_VALUE = IM_COL32(255, 255, 255, 255);
    static constexpr ImU32 COLOR_GOOD = IM_COL32(100, 255, 100, 255);
    static constexpr ImU32 COLOR_WARNING = IM_COL32(255, 200, 50, 255);
    static constexpr ImU32 COLOR_DANGER = IM_COL32(255, 80, 80, 255);
    static constexpr ImU32 COLOR_HYPERBOLIC = IM_COL32(150, 150, 255, 255);
};

#endif // ORBITAL_INFO_H
