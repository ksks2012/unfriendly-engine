#include "core/flight_plan.h"

FlightPlan::FlightPlan(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << filename << ". Using defaults." << std::endl;
        return;
    }
    json flightPlan;
    file >> flightPlan;
    parseFlightPlan(flightPlan);
}

FlightPlan::FlightPlan(nlohmann::json& json) {
    parseFlightPlan(json);
}

void FlightPlan::parseFlightPlan(const nlohmann::json& json) {
    if (json.contains("flight_plan") && json["flight_plan"].is_array()) {
        const auto& flight_plan = json["flight_plan"];
        for (const auto& stage_json : flight_plan) {
            FlightStage stage;

            // parse condition
            if (stage_json.contains("condition")) {
                const auto& condition = stage_json["condition"];
                stage.condition.altitude_min = condition.value("altitude_min", 0.0);
                stage.condition.altitude_max = condition.value("altitude_max", 0.0);
                stage.condition.speed_min = condition.value("speed_min", 0.0);
                stage.condition.speed_max = condition.value("speed_max", 0.0);
            }

            // parse action
            if (stage_json.contains("action")) {
                const auto& action = stage_json["action"];
                stage.action.thrust = action.value("thrust", 0.0);
                if (action.contains("direction") && action["direction"].size() == 3) {
                    stage.action.direction = {
                        action["direction"][0].get<double>(),
                        action["direction"][1].get<double>(),
                        action["direction"][2].get<double>()
                    };
                }
            }

            stages.push_back(stage);
        }
    }
}

std::optional<FlightAction> FlightPlan::getAction(double altitude, double speed) const {
    for (const auto& stage : stages) {
        if (stage.condition.isSatisfied(altitude, speed)) {
            return stage.action;
        }
    }
    return std::nullopt;
}

void FlightPlan::addStage(const FlightStage& stage) { \
    stages.push_back(stage);
}

const std::vector<FlightStage>& FlightPlan::getStages() const { 
    return stages;
}
