#ifndef NAVBALL_H
#define NAVBALL_H

#include "core/rocket.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

/**
 * NavBall - A space navigation HUD element similar to KSP's navball
 * 
 * Displays:
 * - Artificial horizon with pitch/roll indication
 * - Prograde/Retrograde markers (velocity direction)
 * - Normal/Anti-normal markers (perpendicular to orbital plane)
 * - Radial in/out markers
 * - Thrust direction indicator
 * - Heading compass
 */
class NavBall {
public:
    NavBall();
    
    /**
     * Render the NavBall HUD
     * @param rocket The rocket to display information for
     * @param earthPos Earth position (for reference frame calculations)
     * @param panelX X position of the panel
     * @param panelY Y position of the panel
     * @param size Size of the navball (diameter)
     */
    void render(const Rocket& rocket, const glm::vec3& earthPos, 
                float panelX, float panelY, float size = 150.0f);

private:
    // Calculate orbital reference frame vectors
    struct OrbitalFrame {
        glm::vec3 prograde;     // Velocity direction (green)
        glm::vec3 retrograde;   // Opposite to velocity (yellow)
        glm::vec3 normal;       // Perpendicular to orbital plane, "up" (magenta)
        glm::vec3 antiNormal;   // Opposite to normal (cyan)
        glm::vec3 radialIn;     // Toward central body (cyan)
        glm::vec3 radialOut;    // Away from central body (cyan)
    };
    
    OrbitalFrame calculateOrbitalFrame(const glm::vec3& position, const glm::vec3& velocity, 
                                        const glm::vec3& centralBodyPos) const;
    
    // Project a 3D direction onto the navball 2D surface
    // Returns (x, y) in navball local coordinates, and isVisible flag
    struct NavBallProjection {
        float x, y;
        bool isVisible;  // Front hemisphere
        float depth;     // For sorting/fading
    };
    
    NavBallProjection projectToNavBall(const glm::vec3& direction, const glm::vec3& rocketUp,
                                        const glm::vec3& rocketForward, const glm::vec3& rocketRight,
                                        float radius) const;
    
    // Draw a marker on the navball
    void drawMarker(ImDrawList* drawList, ImVec2 center, const NavBallProjection& proj,
                    ImU32 color, const char* label, bool filled = true) const;
    
    // Draw the artificial horizon
    void drawHorizon(ImDrawList* drawList, ImVec2 center, float radius,
                     float pitch, float roll) const;
    
    // Draw heading compass
    void drawCompass(ImDrawList* drawList, ImVec2 center, float radius, float heading) const;
    
    // Draw the fixed aircraft symbol (center reference)
    void drawAircraftSymbol(ImDrawList* drawList, ImVec2 center, float radius) const;
    
    // Calculate pitch, roll, heading from rocket orientation
    void calculateAttitude(const glm::vec3& thrustDir, const glm::vec3& velocity,
                           const glm::vec3& radialOut, float& pitch, float& roll, float& heading) const;
    
    // Calculate angle between two vectors in degrees
    float angleBetween(const glm::vec3& a, const glm::vec3& b) const;
    
    // Colors for different markers
    static constexpr ImU32 COLOR_PROGRADE = IM_COL32(0, 255, 0, 255);      // Green
    static constexpr ImU32 COLOR_RETROGRADE = IM_COL32(255, 255, 0, 255);  // Yellow
    static constexpr ImU32 COLOR_NORMAL = IM_COL32(255, 0, 255, 255);      // Magenta
    static constexpr ImU32 COLOR_ANTINORMAL = IM_COL32(255, 0, 255, 180);  // Magenta (faded)
    static constexpr ImU32 COLOR_RADIAL_IN = IM_COL32(0, 255, 255, 255);   // Cyan
    static constexpr ImU32 COLOR_RADIAL_OUT = IM_COL32(0, 255, 255, 180);  // Cyan (faded)
    static constexpr ImU32 COLOR_THRUST = IM_COL32(255, 128, 0, 255);      // Orange
    static constexpr ImU32 COLOR_SKY = IM_COL32(50, 100, 180, 255);        // Blue sky
    static constexpr ImU32 COLOR_GROUND = IM_COL32(100, 70, 40, 255);      // Brown ground
    static constexpr ImU32 COLOR_HORIZON = IM_COL32(255, 255, 255, 255);   // White horizon line
    static constexpr ImU32 COLOR_COMPASS = IM_COL32(200, 200, 200, 255);   // Light gray
    static constexpr ImU32 COLOR_ALIGNED = IM_COL32(0, 255, 100, 255);     // Green for aligned
    static constexpr ImU32 COLOR_WARNING = IM_COL32(255, 100, 0, 255);     // Orange for misaligned
};

#endif // NAVBALL_H
