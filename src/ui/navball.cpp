#include "ui/navball.h"

#include <cmath>
#include <algorithm>

NavBall::NavBall() {}

NavBall::OrbitalFrame NavBall::calculateOrbitalFrame(const glm::vec3& position, 
                                                      const glm::vec3& velocity,
                                                      const glm::vec3& centralBodyPos) const {
    OrbitalFrame frame;
    
    // Radial vector: from central body to spacecraft
    glm::vec3 radial = position - centralBodyPos;
    float radialLen = glm::length(radial);
    
    if (radialLen < 0.001f) {
        // Fallback if too close to center
        frame.radialOut = glm::vec3(0.0f, 1.0f, 0.0f);
    } else {
        frame.radialOut = glm::normalize(radial);
    }
    frame.radialIn = -frame.radialOut;
    
    // Prograde: velocity direction
    float velLen = glm::length(velocity);
    if (velLen < 0.001f) {
        // If no velocity, use a default direction
        frame.prograde = glm::vec3(1.0f, 0.0f, 0.0f);
    } else {
        frame.prograde = glm::normalize(velocity);
    }
    frame.retrograde = -frame.prograde;
    
    // Normal: perpendicular to orbital plane (radial × prograde)
    glm::vec3 normal = glm::cross(frame.radialOut, frame.prograde);
    float normalLen = glm::length(normal);
    if (normalLen < 0.001f) {
        // Velocity is purely radial, use arbitrary normal
        frame.normal = glm::vec3(0.0f, 0.0f, 1.0f);
    } else {
        frame.normal = glm::normalize(normal);
    }
    frame.antiNormal = -frame.normal;
    
    return frame;
}

NavBall::NavBallProjection NavBall::projectToNavBall(const glm::vec3& direction,
                                                      const glm::vec3& rocketUp,
                                                      const glm::vec3& rocketForward,
                                                      const glm::vec3& rocketRight,
                                                      float radius) const {
    NavBallProjection proj;
    
    // Normalize the direction
    glm::vec3 dir = glm::normalize(direction);
    
    // Project onto rocket's local coordinate system
    // Forward = into the screen (Z), Right = X, Up = Y
    float x = glm::dot(dir, rocketRight);
    float y = glm::dot(dir, rocketUp);
    float z = glm::dot(dir, rocketForward);  // Depth (positive = front)
    
    // Convert to navball coordinates
    // Use stereographic-like projection for better visualization
    proj.depth = z;
    proj.isVisible = (z > -0.1f);  // Slightly behind is still visible
    
    // Scale based on depth for perspective effect
    float scale = 1.0f;
    if (z < 0) {
        // Behind: compress toward edge
        scale = 0.8f + 0.2f * (z + 1.0f);  // Ranges from 0.8 to 1.0
    }
    
    // Project to 2D
    float dist2D = std::sqrt(x * x + y * y);
    if (dist2D > 0.001f) {
        float angle = std::acos(std::clamp(z, -1.0f, 1.0f));
        float projDist = (angle / 3.14159f) * radius * scale;
        proj.x = (x / dist2D) * projDist;
        proj.y = (y / dist2D) * projDist;
    } else {
        proj.x = 0;
        proj.y = 0;
    }
    
    return proj;
}

void NavBall::drawMarker(ImDrawList* drawList, ImVec2 center, const NavBallProjection& proj,
                          ImU32 color, const char* label, bool filled) const {
    if (!proj.isVisible) return;
    
    ImVec2 pos(center.x + proj.x, center.y - proj.y);  // Y is inverted in screen space
    
    // Fade based on depth
    float alpha = proj.depth > 0 ? 1.0f : (0.5f + 0.5f * (proj.depth + 1.0f));
    ImU32 fadedColor = (color & 0x00FFFFFF) | (static_cast<ImU32>(alpha * 255) << 24);
    
    float markerSize = 8.0f * (0.7f + 0.3f * std::max(0.0f, proj.depth));
    
    if (filled) {
        // Draw filled circle with outline
        drawList->AddCircleFilled(pos, markerSize, fadedColor);
        drawList->AddCircle(pos, markerSize, IM_COL32(255, 255, 255, static_cast<int>(alpha * 200)), 0, 2.0f);
    } else {
        // Draw hollow circle
        drawList->AddCircle(pos, markerSize, fadedColor, 0, 2.0f);
    }
    
    // Draw label
    if (label && label[0] != '\0') {
        ImVec2 textPos(pos.x + markerSize + 3, pos.y - 6);
        drawList->AddText(textPos, fadedColor, label);
    }
}

void NavBall::drawHorizon(ImDrawList* drawList, ImVec2 center, float radius,
                           float pitch, float roll) const {
    // Convert angles to radians
    float pitchRad = glm::radians(pitch);
    float rollRad = glm::radians(roll);
    
    // Calculate horizon line offset based on pitch
    // Pitch range: -90 to +90 degrees maps to -radius to +radius
    float horizonOffset = (pitch / 90.0f) * radius;
    
    // Draw clipped sky and ground
    drawList->PushClipRect(ImVec2(center.x - radius, center.y - radius),
                            ImVec2(center.x + radius, center.y + radius), true);
    
    // Calculate rotated horizon line endpoints
    float cosRoll = std::cos(rollRad);
    float sinRoll = std::sin(rollRad);
    
    // Horizon line points (extended beyond circle for rotation)
    float lineLen = radius * 2.0f;
    ImVec2 horizonLeft(center.x - lineLen * cosRoll, 
                       center.y + horizonOffset - lineLen * sinRoll);
    ImVec2 horizonRight(center.x + lineLen * cosRoll, 
                        center.y + horizonOffset + lineLen * sinRoll);
    
    // Draw sky (above horizon)
    ImVec2 skyPoly[4];
    skyPoly[0] = ImVec2(center.x - radius * 1.5f, center.y - radius * 1.5f);
    skyPoly[1] = ImVec2(center.x + radius * 1.5f, center.y - radius * 1.5f);
    skyPoly[2] = horizonRight;
    skyPoly[3] = horizonLeft;
    drawList->AddConvexPolyFilled(skyPoly, 4, COLOR_SKY);
    
    // Draw ground (below horizon)
    ImVec2 groundPoly[4];
    groundPoly[0] = horizonLeft;
    groundPoly[1] = horizonRight;
    groundPoly[2] = ImVec2(center.x + radius * 1.5f, center.y + radius * 1.5f);
    groundPoly[3] = ImVec2(center.x - radius * 1.5f, center.y + radius * 1.5f);
    drawList->AddConvexPolyFilled(groundPoly, 4, COLOR_GROUND);
    
    // Draw horizon line
    drawList->AddLine(horizonLeft, horizonRight, COLOR_HORIZON, 2.0f);
    
    // Draw pitch ladder lines
    for (int pitchLine = -80; pitchLine <= 80; pitchLine += 10) {
        if (pitchLine == 0) continue;  // Skip horizon line
        
        float lineOffset = ((pitch - pitchLine) / 90.0f) * radius;
        if (std::abs(lineOffset) > radius) continue;  // Off screen
        
        float ladderLen = (pitchLine % 30 == 0) ? 30.0f : 15.0f;
        
        ImVec2 left(center.x - ladderLen * cosRoll, 
                    center.y + lineOffset - ladderLen * sinRoll);
        ImVec2 right(center.x + ladderLen * cosRoll, 
                     center.y + lineOffset + ladderLen * sinRoll);
        
        ImU32 ladderColor = (pitchLine > 0) ? IM_COL32(100, 150, 255, 180) : IM_COL32(200, 150, 100, 180);
        drawList->AddLine(left, right, ladderColor, 1.5f);
        
        // Add pitch value text for major lines
        if (pitchLine % 30 == 0) {
            char pitchText[8];
            snprintf(pitchText, sizeof(pitchText), "%d", std::abs(pitchLine));
            ImVec2 textPos(right.x + 5, right.y - 6);
            drawList->AddText(textPos, ladderColor, pitchText);
        }
    }
    
    drawList->PopClipRect();
    
    // Draw navball border
    drawList->AddCircle(center, radius, IM_COL32(80, 80, 80, 255), 0, 3.0f);
    drawList->AddCircle(center, radius + 2, IM_COL32(40, 40, 40, 255), 0, 2.0f);
}

void NavBall::drawCompass(ImDrawList* drawList, ImVec2 center, float radius, float heading) const {
    float compassRadius = radius + 15.0f;
    
    // Draw compass ring
    drawList->AddCircle(center, compassRadius, COLOR_COMPASS, 0, 1.5f);
    
    // Draw cardinal directions
    const char* cardinals[] = {"N", "E", "S", "W"};
    float cardinalAngles[] = {0.0f, 90.0f, 180.0f, 270.0f};
    
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(cardinalAngles[i] - heading - 90.0f);  // -90 to put N at top
        float x = center.x + compassRadius * std::cos(angle);
        float y = center.y + compassRadius * std::sin(angle);
        
        ImU32 color = (i == 0) ? IM_COL32(255, 50, 50, 255) : COLOR_COMPASS;  // N is red
        drawList->AddText(ImVec2(x - 4, y - 6), color, cardinals[i]);
    }
    
    // Draw tick marks
    for (int deg = 0; deg < 360; deg += 15) {
        if (deg % 90 == 0) continue;  // Skip cardinals
        
        float angle = glm::radians(static_cast<float>(deg) - heading - 90.0f);
        float innerR = compassRadius - 5.0f;
        float outerR = compassRadius;
        
        if (deg % 45 == 0) {
            innerR -= 3.0f;  // Longer tick for 45° marks
        }
        
        ImVec2 inner(center.x + innerR * std::cos(angle), center.y + innerR * std::sin(angle));
        ImVec2 outer(center.x + outerR * std::cos(angle), center.y + outerR * std::sin(angle));
        drawList->AddLine(inner, outer, COLOR_COMPASS, 1.0f);
    }
    
    // Draw heading indicator at top
    ImVec2 indicatorTop(center.x, center.y - compassRadius - 10);
    ImVec2 indicatorLeft(center.x - 6, center.y - compassRadius - 2);
    ImVec2 indicatorRight(center.x + 6, center.y - compassRadius - 2);
    drawList->AddTriangleFilled(indicatorTop, indicatorLeft, indicatorRight, IM_COL32(255, 200, 0, 255));
}

void NavBall::drawAircraftSymbol(ImDrawList* drawList, ImVec2 center, float radius) const {
    // Draw a fixed aircraft symbol at center
    ImU32 symbolColor = IM_COL32(255, 200, 0, 255);  // Yellow/orange
    float wingSpan = 25.0f;
    float bodyLen = 8.0f;
    
    // Wings
    drawList->AddLine(ImVec2(center.x - wingSpan, center.y), 
                      ImVec2(center.x - 8, center.y), symbolColor, 3.0f);
    drawList->AddLine(ImVec2(center.x + 8, center.y), 
                      ImVec2(center.x + wingSpan, center.y), symbolColor, 3.0f);
    
    // Body
    drawList->AddLine(ImVec2(center.x, center.y - bodyLen), 
                      ImVec2(center.x, center.y + bodyLen), symbolColor, 3.0f);
    
    // Center dot
    drawList->AddCircleFilled(center, 4.0f, symbolColor);
}

void NavBall::calculateAttitude(const glm::vec3& thrustDir, const glm::vec3& velocity,
                                 const glm::vec3& radialOut, float& pitch, float& roll, float& heading) const {
    // Use radial out as the "up" reference
    glm::vec3 up = glm::normalize(radialOut);
    
    // Calculate heading (angle in horizontal plane)
    // Project thrust direction onto horizontal plane
    glm::vec3 thrustHoriz = thrustDir - glm::dot(thrustDir, up) * up;
    float horizLen = glm::length(thrustHoriz);
    
    if (horizLen > 0.001f) {
        thrustHoriz = glm::normalize(thrustHoriz);
        // Use a reference direction (e.g., velocity direction projected to horizontal)
        glm::vec3 refDir = velocity - glm::dot(velocity, up) * up;
        float refLen = glm::length(refDir);
        
        if (refLen > 0.001f) {
            refDir = glm::normalize(refDir);
            heading = glm::degrees(std::atan2(glm::dot(glm::cross(refDir, thrustHoriz), up),
                                              glm::dot(refDir, thrustHoriz)));
            if (heading < 0) heading += 360.0f;
        } else {
            heading = 0.0f;
        }
    } else {
        heading = 0.0f;
    }
    
    // Calculate pitch (angle from horizontal)
    pitch = glm::degrees(std::asin(std::clamp(glm::dot(thrustDir, up), -1.0f, 1.0f)));
    
    // Roll calculation (simplified - based on the relationship between velocity and thrust)
    // In a proper system, this would use gyroscope data
    roll = 0.0f;  // Simplified: no roll calculation for now
}

float NavBall::angleBetween(const glm::vec3& a, const glm::vec3& b) const {
    float lenA = glm::length(a);
    float lenB = glm::length(b);
    
    if (lenA < 0.001f || lenB < 0.001f) {
        return 0.0f;
    }
    
    float cosAngle = glm::dot(a, b) / (lenA * lenB);
    cosAngle = std::clamp(cosAngle, -1.0f, 1.0f);
    return glm::degrees(std::acos(cosAngle));
}

void NavBall::render(const Rocket& rocket, const glm::vec3& earthPos,
                      float panelX, float panelY, float size) {
    ImGui::SetNextWindowPos(ImVec2(panelX, panelY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(size + 60, size + 140), ImGuiCond_Always);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                             ImGuiWindowFlags_NoCollapse;
    
    ImGui::Begin("NavBall", nullptr, flags);
    
    // Get rocket data
    glm::vec3 position = rocket.getPosition();
    glm::vec3 velocity = rocket.getVelocity();
    glm::vec3 thrustDir = rocket.getThrustDirection();
    
    // Calculate orbital reference frame
    OrbitalFrame frame = calculateOrbitalFrame(position, velocity, earthPos);
    
    // Define rocket's local coordinate system based on thrust direction
    glm::vec3 rocketForward = glm::normalize(thrustDir);  // Rocket points in thrust direction
    
    // Choose an "up" reference - use radial out
    glm::vec3 tempUp = frame.radialOut;
    glm::vec3 rocketRight = glm::cross(rocketForward, tempUp);
    float rightLen = glm::length(rocketRight);
    if (rightLen < 0.001f) {
        // Thrust is radial, use velocity as reference
        tempUp = glm::length(velocity) > 0.001f ? glm::normalize(velocity) : glm::vec3(1, 0, 0);
        rocketRight = glm::cross(rocketForward, tempUp);
    }
    rocketRight = glm::normalize(rocketRight);
    glm::vec3 rocketUp = glm::normalize(glm::cross(rocketRight, rocketForward));
    
    // Calculate attitude
    float pitch, roll, heading;
    calculateAttitude(thrustDir, velocity, frame.radialOut, pitch, roll, heading);
    
    // Get draw list and center position
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    float radius = size * 0.4f;
    ImVec2 center(windowPos.x + size / 2 + 30, windowPos.y + size / 2 + 35);
    
    // Draw horizon
    drawHorizon(drawList, center, radius, pitch, roll);
    
    // Draw compass
    drawCompass(drawList, center, radius, heading);
    
    // Project and draw orbital markers
    NavBallProjection progradeProj = projectToNavBall(frame.prograde, rocketUp, rocketForward, rocketRight, radius);
    NavBallProjection retrogradeProj = projectToNavBall(frame.retrograde, rocketUp, rocketForward, rocketRight, radius);
    NavBallProjection normalProj = projectToNavBall(frame.normal, rocketUp, rocketForward, rocketRight, radius);
    NavBallProjection antiNormalProj = projectToNavBall(frame.antiNormal, rocketUp, rocketForward, rocketRight, radius);
    NavBallProjection radialInProj = projectToNavBall(frame.radialIn, rocketUp, rocketForward, rocketRight, radius);
    NavBallProjection radialOutProj = projectToNavBall(frame.radialOut, rocketUp, rocketForward, rocketRight, radius);
    
    // Draw markers
    drawMarker(drawList, center, progradeProj, COLOR_PROGRADE, "Pro");
    drawMarker(drawList, center, retrogradeProj, COLOR_RETROGRADE, "Ret", false);
    drawMarker(drawList, center, normalProj, COLOR_NORMAL, "Nrm");
    drawMarker(drawList, center, antiNormalProj, COLOR_ANTINORMAL, "A-N", false);
    drawMarker(drawList, center, radialInProj, COLOR_RADIAL_IN, "R-", false);
    drawMarker(drawList, center, radialOutProj, COLOR_RADIAL_OUT, "R+", false);
    
    // Draw thrust direction marker
    NavBallProjection thrustProj = projectToNavBall(thrustDir, rocketUp, rocketForward, rocketRight, radius);
    // Thrust is always at center since rocket frame is based on thrust direction
    drawList->AddCircleFilled(center, 6.0f, COLOR_THRUST);
    drawList->AddCircle(center, 6.0f, IM_COL32(255, 255, 255, 200), 0, 2.0f);
    
    // Draw aircraft symbol
    drawAircraftSymbol(drawList, center, radius);
    
    // Calculate thrust-prograde alignment
    float alignmentAngle = angleBetween(thrustDir, frame.prograde);
    
    // Draw info text below navball
    ImGui::SetCursorPosY(size + 45);
    
    // Speed display
    float speed = glm::length(velocity);
    if (speed > 1000.0f) {
        ImGui::Text("Speed: %.2f km/s", speed / 1000.0f);
    } else {
        ImGui::Text("Speed: %.1f m/s", speed);
    }
    
    // Altitude (distance from Earth)
    float altitude = glm::length(position - earthPos) - 6371000.0f;  // Subtract Earth radius
    if (altitude > 1000000.0f) {
        ImGui::Text("Alt: %.0f km", altitude / 1000.0f);
    } else if (altitude > 1000.0f) {
        ImGui::Text("Alt: %.1f km", altitude / 1000.0f);
    } else {
        ImGui::Text("Alt: %.0f m", altitude);
    }
    
    // Pitch and Heading
    ImGui::Text("Pitch: %.1f deg", pitch);
    ImGui::Text("Hdg: %.0f deg", heading);
    
    // Alignment indicator
    ImGui::Separator();
    if (alignmentAngle < 5.0f) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.4f, 1.0f), "ALIGNED (%.1f)", alignmentAngle);
    } else if (alignmentAngle < 15.0f) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Align: %.1f deg", alignmentAngle);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), "Align: %.1f deg", alignmentAngle);
    }
    
    ImGui::End();
}
