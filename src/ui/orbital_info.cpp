#include "ui/orbital_info.h"
#include "core/body.h"

#include <algorithm>
#include <cmath>

void OrbitalInfo::render(const Rocket& rocket, const BODY_MAP& bodies,
                          float panelX, float panelY) {
    ImGui::SetNextWindowPos(ImVec2(panelX, panelY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(220.0f, 320.0f), ImGuiCond_Always);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse;
    
    ImGui::Begin("Orbital Info", nullptr, flags);
    
    // Get rocket state
    glm::dvec3 rocketPos = glm::dvec3(rocket.getPosition());
    glm::dvec3 rocketVel = glm::dvec3(rocket.getVelocity());
    
    // Find the dominant body (the one we're orbiting)
    std::string dominantBodyName = findDominantBody(rocketPos, bodies);
    
    if (dominantBodyName.empty() || bodies.find(dominantBodyName) == bodies.end()) {
        ImGui::Text("No reference body found");
        ImGui::End();
        return;
    }
    
    const auto& centralBody = bodies.at(dominantBodyName);
    glm::dvec3 centralPos = glm::dvec3(centralBody->position);
    
    // Calculate position and velocity relative to central body
    glm::dvec3 relPos = rocketPos - centralPos;
    glm::dvec3 relVel = rocketVel - glm::dvec3(centralBody->velocity);
    
    // Calculate orbital elements
    OrbitalElements elements = OrbitalCalculator::calculate(
        relPos, relVel,
        centralBody->mass,
        6371000.0,  // Default radius, will be updated per body
        dominantBodyName
    );
    
    // Update radius based on body
    double bodyRadius = 6371000.0;  // Earth default
    if (dominantBodyName == "earth") bodyRadius = 6371000.0;
    else if (dominantBodyName == "moon") bodyRadius = 1737400.0;
    else if (dominantBodyName == "sun") bodyRadius = 696340000.0;
    else if (dominantBodyName == "mercury") bodyRadius = 2439700.0;
    else if (dominantBodyName == "venus") bodyRadius = 6051800.0;
    else if (dominantBodyName == "mars") bodyRadius = 3389500.0;
    else if (dominantBodyName == "jupiter") bodyRadius = 69911000.0;
    else if (dominantBodyName == "saturn") bodyRadius = 58232000.0;
    else if (dominantBodyName == "uranus") bodyRadius = 25362000.0;
    else if (dominantBodyName == "neptune") bodyRadius = 24622000.0;
    
    // Recalculate with correct radius
    elements = OrbitalCalculator::calculate(relPos, relVel, centralBody->mass, bodyRadius, dominantBodyName);
    
    // Header: Reference body and orbit type
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Orbiting: %s", 
                       dominantBodyName.c_str());
    
    // Orbit type with color coding
    ImVec4 orbitColor;
    switch (elements.orbitType) {
        case OrbitalElements::OrbitType::Circular:
            orbitColor = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
            break;
        case OrbitalElements::OrbitType::Elliptical:
            orbitColor = ImVec4(0.4f, 1.0f, 0.8f, 1.0f);
            break;
        case OrbitalElements::OrbitType::Hyperbolic:
            orbitColor = ImVec4(0.6f, 0.6f, 1.0f, 1.0f);
            break;
        case OrbitalElements::OrbitType::Parabolic:
            orbitColor = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
            break;
        case OrbitalElements::OrbitType::Suborbital:
        default:
            orbitColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            break;
    }
    ImGui::TextColored(orbitColor, "Type: %s", elements.getOrbitTypeString());
    
    ImGui::Separator();
    
    // Current state
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Current State:");
    ImGui::Text("Altitude: %s", OrbitalElements::formatDistance(elements.altitude).c_str());
    ImGui::Text("Speed: %s", OrbitalElements::formatVelocity(elements.speed).c_str());
    
    ImGui::Separator();
    
    // Apsides
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Apsides:");
    
    // Periapsis with color based on danger
    ImVec4 periColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    if (elements.periapsisAltitude < 0) {
        periColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);  // Red - will impact
    } else if (elements.periapsisAltitude < 100000) {
        periColor = ImVec4(1.0f, 0.8f, 0.3f, 1.0f);  // Yellow - low
    }
    ImGui::TextColored(periColor, "Periapsis: %s", 
                       OrbitalElements::formatDistance(elements.periapsisAltitude).c_str());
    
    // Apoapsis
    if (std::isfinite(elements.apoapsisAltitude)) {
        ImGui::Text("Apoapsis: %s", OrbitalElements::formatDistance(elements.apoapsisAltitude).c_str());
    } else {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), "Apoapsis: Escape");
    }
    
    ImGui::Separator();
    
    // Orbital elements
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Orbital Elements:");
    
    // Semi-major axis
    if (std::isfinite(elements.semiMajorAxis) && elements.semiMajorAxis > 0) {
        ImGui::Text("Semi-major: %s", OrbitalElements::formatDistance(elements.semiMajorAxis).c_str());
    } else if (elements.semiMajorAxis < 0) {
        ImGui::Text("Semi-major: %s", OrbitalElements::formatDistance(elements.semiMajorAxis).c_str());
    } else {
        ImGui::Text("Semi-major: Infinite");
    }
    
    // Eccentricity with visual bar
    char eccBuf[32];
    snprintf(eccBuf, sizeof(eccBuf), "%.4f", elements.eccentricity);
    
    ImVec4 eccColor;
    if (elements.eccentricity < 0.01) {
        eccColor = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);  // Green - circular
    } else if (elements.eccentricity < 1.0) {
        eccColor = ImVec4(0.4f, 0.8f, 1.0f, 1.0f);  // Cyan - elliptical
    } else {
        eccColor = ImVec4(0.6f, 0.6f, 1.0f, 1.0f);  // Purple - hyperbolic
    }
    ImGui::TextColored(eccColor, "Eccentricity: %s", eccBuf);
    
    // Inclination
    ImGui::Text("Inclination: %.2f deg", elements.inclination);
    
    // Orbital period
    if (elements.isClosed()) {
        ImGui::Text("Period: %s", OrbitalElements::formatTime(elements.orbitalPeriod).c_str());
    } else {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Period: N/A (open orbit)");
    }
    
    ImGui::Separator();
    
    // Advanced elements (collapsible)
    if (ImGui::TreeNode("Advanced")) {
        ImGui::Text("LAN: %.2f deg", elements.longitudeOfAscendingNode);
        ImGui::Text("Arg. of Pe: %.2f deg", elements.argumentOfPeriapsis);
        ImGui::Text("True Anomaly: %.2f deg", elements.trueAnomaly);
        ImGui::Text("Spec. Energy: %.2e J/kg", elements.specificOrbitalEnergy);
        ImGui::Text("Ang. Momentum: %.2e m2/s", elements.specificAngularMomentum);
        ImGui::TreePop();
    }
    
    ImGui::End();
}

std::string OrbitalInfo::findDominantBody(const glm::dvec3& position, const BODY_MAP& bodies) const {
    // Simple sphere of influence check
    // SOI ≈ a * (m/M)^(2/5) where a is semi-major axis, m is body mass, M is parent mass
    
    std::string dominant = "";
    double minInfluence = std::numeric_limits<double>::max();
    
    // First, check if we're in any moon's SOI
    if (bodies.find("moon") != bodies.end() && bodies.find("earth") != bodies.end()) {
        const auto& moon = bodies.at("moon");
        const auto& earth = bodies.at("earth");
        
        double moonDist = glm::length(position - glm::dvec3(moon->position));
        double earthMoonDist = glm::length(glm::dvec3(moon->position) - glm::dvec3(earth->position));
        
        // Moon's SOI relative to Earth
        double moonSOI = earthMoonDist * std::pow(moon->mass / earth->mass, 0.4);
        
        if (moonDist < moonSOI) {
            return "moon";
        }
    }
    
    // Check planets' SOI relative to Sun
    if (bodies.find("sun") != bodies.end()) {
        const auto& sun = bodies.at("sun");
        
        std::vector<std::string> planets = {"mercury", "venus", "earth", "mars", 
                                             "jupiter", "saturn", "uranus", "neptune"};
        
        for (const auto& planetName : planets) {
            if (bodies.find(planetName) == bodies.end()) continue;
            
            const auto& planet = bodies.at(planetName);
            double planetDist = glm::length(position - glm::dvec3(planet->position));
            double sunPlanetDist = glm::length(glm::dvec3(planet->position) - glm::dvec3(sun->position));
            
            // Planet's SOI relative to Sun
            double planetSOI = sunPlanetDist * std::pow(planet->mass / sun->mass, 0.4);
            
            if (planetDist < planetSOI && planetDist < minInfluence) {
                minInfluence = planetDist;
                dominant = planetName;
            }
        }
        
        // If not in any planet's SOI, use Sun
        if (dominant.empty()) {
            dominant = "sun";
        }
    } else {
        // Fallback: use Earth
        dominant = "earth";
    }
    
    return dominant;
}

void OrbitalInfo::renderElement(const char* label, const std::string& value, ImU32 color) const {
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s:", label);
    ImGui::SameLine();
    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(color), "%s", value.c_str());
}

void OrbitalInfo::renderProgressBar(const char* label, float value, float maxValue,
                                     const char* overlay, ImU32 color) const {
    ImGui::Text("%s:", label);
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
    ImGui::ProgressBar(std::min(value / maxValue, 1.0f), ImVec2(-1, 0), overlay);
    ImGui::PopStyleColor();
}
