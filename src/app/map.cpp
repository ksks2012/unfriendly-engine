#include "app/map.h"
#include <imgui.h>
#include <glm/gtx/string_cast.hpp>

Map::Map(Simulation& sim) : simulation(sim), mapClicked(false), lastClickPos(0.0f) {}

void Map::render(int width, int height) {
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(200, 200));
    ImGui::Begin("Map View", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    float mapWidth = contentSize.x;
    float mapHeight = contentSize.y;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    float cx = canvasPos.x + mapWidth * 0.5f;
    float cy = canvasPos.y + mapHeight * 0.5f;

    double mapScale = 0.000000001;
    drawList->AddCircleFilled(ImVec2(cx, cy), 8.0f, IM_COL32(0, 0, 150, 255));

    glm::dvec3 moonPos = simulation.getMoonPos();
    float mx = cx + static_cast<float>(moonPos.x * mapScale * 100.0);
    float my = cy + static_cast<float>(moonPos.y * mapScale * 100.0);
    drawList->AddCircleFilled(ImVec2(mx, my), 3.0f, IM_COL32(200, 200, 200, 255));

    glm::dvec3 rocketPos = simulation.getRocket().getPosition();
    float rx = cx + static_cast<float>(rocketPos.x * mapScale * 100.0);
    float ry = cy + static_cast<float>(rocketPos.y * mapScale * 100.0);
    drawList->AddCircleFilled(ImVec2(rx, ry), 2.0f, IM_COL32(255, 0, 0, 255));

    ImGui::End();
}