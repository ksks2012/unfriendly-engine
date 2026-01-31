#ifndef ORBITAL_ELEMENTS_H
#define ORBITAL_ELEMENTS_H

#include <glm/glm.hpp>
#include <string>
#include <cmath>
#include <algorithm>
#include <limits>

/**
 * OrbitalElements - Calculates and stores Keplerian orbital elements
 * 
 * Reference frame: Central body at origin
 * Orbital plane: XZ plane for zero inclination
 */
struct OrbitalElements {
    // Primary orbital elements
    double semiMajorAxis;      // a - Semi-major axis (meters)
    double eccentricity;       // e - Eccentricity (0 = circular, <1 = ellipse, 1 = parabola, >1 = hyperbola)
    double inclination;        // i - Inclination (degrees)
    double longitudeOfAscendingNode; // Ω - Longitude of ascending node (degrees)
    double argumentOfPeriapsis;      // ω - Argument of periapsis (degrees)
    double trueAnomaly;        // ν - True anomaly (degrees)
    
    // Derived elements
    double periapsis;          // Periapsis distance from center (meters)
    double apoapsis;           // Apoapsis distance from center (meters) - negative for hyperbolic
    double periapsisAltitude;  // Periapsis altitude above surface (meters)
    double apoapsisAltitude;   // Apoapsis altitude above surface (meters)
    double orbitalPeriod;      // Orbital period (seconds) - NaN for non-elliptical
    double meanMotion;         // Mean motion (radians/second)
    double specificOrbitalEnergy; // Specific orbital energy (J/kg)
    double specificAngularMomentum; // Specific angular momentum magnitude (m²/s)
    
    // Current state
    double speed;              // Current orbital speed (m/s)
    double altitude;           // Current altitude above surface (meters)
    double distance;           // Current distance from center (meters)
    
    // Reference body info
    double centralBodyMass;    // Mass of central body (kg)
    double centralBodyRadius;  // Radius of central body (meters)
    std::string centralBodyName; // Name of the central body
    
    // Orbit classification
    enum class OrbitType {
        Suborbital,    // Will impact surface
        Circular,      // e ≈ 0
        Elliptical,    // 0 < e < 1
        Parabolic,     // e ≈ 1
        Hyperbolic     // e > 1
    };
    OrbitType orbitType;
    
    // Default constructor
    OrbitalElements() 
        : semiMajorAxis(0), eccentricity(0), inclination(0),
          longitudeOfAscendingNode(0), argumentOfPeriapsis(0), trueAnomaly(0),
          periapsis(0), apoapsis(0), periapsisAltitude(0), apoapsisAltitude(0),
          orbitalPeriod(0), meanMotion(0), specificOrbitalEnergy(0), specificAngularMomentum(0),
          speed(0), altitude(0), distance(0),
          centralBodyMass(0), centralBodyRadius(0), centralBodyName(""),
          orbitType(OrbitType::Suborbital) {}
    
    /**
     * Get orbit type as string
     */
    const char* getOrbitTypeString() const {
        switch (orbitType) {
            case OrbitType::Suborbital: return "Suborbital";
            case OrbitType::Circular: return "Circular";
            case OrbitType::Elliptical: return "Elliptical";
            case OrbitType::Parabolic: return "Parabolic";
            case OrbitType::Hyperbolic: return "Hyperbolic";
            default: return "Unknown";
        }
    }
    
    /**
     * Check if orbit is closed (elliptical)
     */
    bool isClosed() const {
        return eccentricity < 1.0 && orbitType != OrbitType::Suborbital;
    }
    
    /**
     * Format time in human readable format
     */
    static std::string formatTime(double seconds) {
        if (!std::isfinite(seconds) || seconds < 0) {
            return "N/A";
        }
        
        int days = static_cast<int>(seconds / 86400);
        seconds -= days * 86400;
        int hours = static_cast<int>(seconds / 3600);
        seconds -= hours * 3600;
        int minutes = static_cast<int>(seconds / 60);
        seconds -= minutes * 60;
        
        char buffer[64];
        if (days > 0) {
            snprintf(buffer, sizeof(buffer), "%dd %dh %dm", days, hours, minutes);
        } else if (hours > 0) {
            snprintf(buffer, sizeof(buffer), "%dh %dm %.0fs", hours, minutes, seconds);
        } else if (minutes > 0) {
            snprintf(buffer, sizeof(buffer), "%dm %.1fs", minutes, seconds);
        } else {
            snprintf(buffer, sizeof(buffer), "%.1fs", seconds);
        }
        return std::string(buffer);
    }
    
    /**
     * Format distance in appropriate units
     */
    static std::string formatDistance(double meters) {
        if (!std::isfinite(meters)) {
            return "N/A";
        }
        
        char buffer[64];
        double absMet = std::abs(meters);
        const char* sign = meters < 0 ? "-" : "";
        
        if (absMet >= 1e12) {
            snprintf(buffer, sizeof(buffer), "%s%.3f AU", sign, absMet / 1.496e11);
        } else if (absMet >= 1e9) {
            snprintf(buffer, sizeof(buffer), "%s%.2f Gm", sign, absMet / 1e9);
        } else if (absMet >= 1e6) {
            snprintf(buffer, sizeof(buffer), "%s%.2f Mm", sign, absMet / 1e6);
        } else if (absMet >= 1e3) {
            snprintf(buffer, sizeof(buffer), "%s%.2f km", sign, absMet / 1e3);
        } else {
            snprintf(buffer, sizeof(buffer), "%s%.1f m", sign, absMet);
        }
        return std::string(buffer);
    }
    
    /**
     * Format velocity
     */
    static std::string formatVelocity(double mps) {
        char buffer[32];
        if (std::abs(mps) >= 1000) {
            snprintf(buffer, sizeof(buffer), "%.3f km/s", mps / 1000.0);
        } else {
            snprintf(buffer, sizeof(buffer), "%.1f m/s", mps);
        }
        return std::string(buffer);
    }
};

/**
 * OrbitalCalculator - Calculates orbital elements from state vectors
 */
class OrbitalCalculator {
public:
    static constexpr double G = 6.67430e-11;  // Gravitational constant
    static constexpr double PI = 3.14159265358979323846;
    static constexpr double DEG_TO_RAD = PI / 180.0;
    static constexpr double RAD_TO_DEG = 180.0 / PI;
    
    /**
     * Calculate orbital elements from position and velocity vectors
     * 
     * @param position Position vector relative to central body (meters)
     * @param velocity Velocity vector (m/s)
     * @param centralBodyMass Mass of central body (kg)
     * @param centralBodyRadius Radius of central body (meters)
     * @param centralBodyName Name of the central body
     * @return OrbitalElements structure with all calculated values
     */
    static OrbitalElements calculate(
        const glm::dvec3& position,
        const glm::dvec3& velocity,
        double centralBodyMass,
        double centralBodyRadius,
        const std::string& centralBodyName = "Central Body"
    ) {
        OrbitalElements elements;
        elements.centralBodyMass = centralBodyMass;
        elements.centralBodyRadius = centralBodyRadius;
        elements.centralBodyName = centralBodyName;
        
        // Standard gravitational parameter
        double mu = G * centralBodyMass;
        
        // Current distance and speed
        double r = glm::length(position);
        double v = glm::length(velocity);
        elements.distance = r;
        elements.speed = v;
        elements.altitude = r - centralBodyRadius;
        
        // Specific angular momentum vector: h = r × v
        glm::dvec3 h = glm::cross(position, velocity);
        double hMag = glm::length(h);
        elements.specificAngularMomentum = hMag;
        
        // Specific orbital energy: ε = v²/2 - μ/r
        double energy = (v * v / 2.0) - (mu / r);
        elements.specificOrbitalEnergy = energy;
        
        // Eccentricity vector: e = ((v² - μ/r)r - (r·v)v) / μ
        glm::dvec3 eVec = ((v * v - mu / r) * position - glm::dot(position, velocity) * velocity) / mu;
        double e = glm::length(eVec);
        elements.eccentricity = e;
        
        // Semi-major axis: a = -μ / (2ε)  for ellipse/hyperbola
        // For parabola (e = 1), a is undefined (infinite)
        double a;
        if (std::abs(energy) < 1e-10) {
            // Parabolic orbit
            a = std::numeric_limits<double>::infinity();
            elements.orbitType = OrbitalElements::OrbitType::Parabolic;
        } else {
            a = -mu / (2.0 * energy);
            elements.semiMajorAxis = a;
            
            if (e < 0.01) {
                elements.orbitType = OrbitalElements::OrbitType::Circular;
            } else if (e < 1.0) {
                elements.orbitType = OrbitalElements::OrbitType::Elliptical;
            } else {
                elements.orbitType = OrbitalElements::OrbitType::Hyperbolic;
            }
        }
        elements.semiMajorAxis = a;
        
        // Periapsis and apoapsis
        if (e < 1.0) {
            // Elliptical orbit
            elements.periapsis = a * (1.0 - e);
            elements.apoapsis = a * (1.0 + e);
        } else if (e > 1.0) {
            // Hyperbolic orbit
            elements.periapsis = a * (1.0 - e);  // Note: a is negative for hyperbola
            elements.apoapsis = std::numeric_limits<double>::infinity();
        } else {
            // Parabolic orbit
            elements.periapsis = hMag * hMag / (2.0 * mu);
            elements.apoapsis = std::numeric_limits<double>::infinity();
        }
        
        // Check for suborbital trajectory
        if (elements.periapsis < centralBodyRadius && elements.orbitType != OrbitalElements::OrbitType::Hyperbolic) {
            elements.orbitType = OrbitalElements::OrbitType::Suborbital;
        }
        
        // Altitudes
        elements.periapsisAltitude = elements.periapsis - centralBodyRadius;
        if (std::isfinite(elements.apoapsis)) {
            elements.apoapsisAltitude = elements.apoapsis - centralBodyRadius;
        } else {
            elements.apoapsisAltitude = std::numeric_limits<double>::infinity();
        }
        
        // Orbital period (only for elliptical orbits)
        if (e < 1.0 && a > 0) {
            elements.orbitalPeriod = 2.0 * PI * std::sqrt(a * a * a / mu);
            elements.meanMotion = std::sqrt(mu / (a * a * a));
        } else {
            elements.orbitalPeriod = std::numeric_limits<double>::quiet_NaN();
            elements.meanMotion = std::numeric_limits<double>::quiet_NaN();
        }
        
        // Inclination: i = acos(hz / |h|)
        // Reference plane is XZ (Y is "up")
        if (hMag > 1e-10) {
            // Using Y as the reference "up" direction
            elements.inclination = std::acos(std::clamp(h.y / hMag, -1.0, 1.0)) * RAD_TO_DEG;
        } else {
            elements.inclination = 0.0;
        }
        
        // Node vector: n = k × h (where k is reference direction, using Y axis)
        glm::dvec3 kVec(0.0, 1.0, 0.0);
        glm::dvec3 n = glm::cross(kVec, h);
        double nMag = glm::length(n);
        
        // Longitude of ascending node
        if (nMag > 1e-10) {
            elements.longitudeOfAscendingNode = std::acos(std::clamp(n.x / nMag, -1.0, 1.0)) * RAD_TO_DEG;
            if (n.z < 0) {
                elements.longitudeOfAscendingNode = 360.0 - elements.longitudeOfAscendingNode;
            }
        } else {
            elements.longitudeOfAscendingNode = 0.0;
        }
        
        // Argument of periapsis
        if (nMag > 1e-10 && e > 1e-10) {
            double dotNE = glm::dot(n, eVec) / (nMag * e);
            elements.argumentOfPeriapsis = std::acos(std::clamp(dotNE, -1.0, 1.0)) * RAD_TO_DEG;
            if (eVec.y < 0) {
                elements.argumentOfPeriapsis = 360.0 - elements.argumentOfPeriapsis;
            }
        } else {
            elements.argumentOfPeriapsis = 0.0;
        }
        
        // True anomaly
        if (e > 1e-10) {
            double dotER = glm::dot(eVec, position) / (e * r);
            elements.trueAnomaly = std::acos(std::clamp(dotER, -1.0, 1.0)) * RAD_TO_DEG;
            if (glm::dot(position, velocity) < 0) {
                elements.trueAnomaly = 360.0 - elements.trueAnomaly;
            }
        } else {
            // Circular orbit - use argument of latitude
            if (nMag > 1e-10) {
                double dotNR = glm::dot(n, position) / (nMag * r);
                elements.trueAnomaly = std::acos(std::clamp(dotNR, -1.0, 1.0)) * RAD_TO_DEG;
                if (position.y < 0) {
                    elements.trueAnomaly = 360.0 - elements.trueAnomaly;
                }
            } else {
                elements.trueAnomaly = 0.0;
            }
        }
        
        return elements;
    }
};

#endif // ORBITAL_ELEMENTS_H
