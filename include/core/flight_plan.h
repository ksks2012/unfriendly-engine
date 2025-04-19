#ifndef FLIGHT_PLAN_H
#define FLIGHT_PLAN_H

#include <iostream>
#include <fstream>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>

using json = nlohmann::json;

struct FlightCondition {
    float altitude_min;
    float altitude_max;
    float speed_min;
    float speed_max;

    // Check if the rocket satisfies the conditions
    bool isSatisfied(float altitude, float speed) const {
        bool altitude_ok = (!altitude_min || altitude >= altitude_min) &&
                          (!altitude_max || altitude <= altitude_max);
        bool speed_ok = (!speed_min || speed >= speed_min) &&
                        (!speed_max || speed <= speed_max);
        return altitude_ok && speed_ok;
    }
};

struct FlightAction {
    float thrust;
    glm::vec3 direction;

    FlightAction(float t = 0.0, glm::vec3 d = {0.0f, 0.0f, 0.0f})
        : thrust(t), direction(d) {}
};

struct FlightStage {
    FlightCondition condition;
    FlightAction action;
};

class FlightPlan {
public:
    FlightPlan() = default;
    FlightPlan(std::string);
    FlightPlan(nlohmann::json&);

    std::optional<FlightAction> getAction(double altitude, double speed) const;

    void addStage(const FlightStage& stage);

    const std::vector<FlightStage>& getStages() const;

private:
    std::vector<FlightStage> stages;
    void parseFlightPlan(const nlohmann::json&);
};

#endif // FLIGHT_PLAN_H