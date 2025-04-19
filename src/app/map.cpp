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

    float mapScale = 0.000000001f;
    drawList->AddCircleFilled(ImVec2(cx, cy), 8.0f, IM_COL32(0, 0, 150, 255));

    glm::vec3 moonPos = simulation.getMoonPos();
    float mx = cx + moonPos.x * mapScale * 100.0f;
    float my = cy + moonPos.y * mapScale * 100.0f;
    drawList->AddCircleFilled(ImVec2(mx, my), 3.0f, IM_COL32(200, 200, 200, 255));

    glm::vec3 rocketPos = simulation.getRocket().getPosition();
    float rx = cx + rocketPos.x * mapScale * 100.0f;
    float ry = cy + rocketPos.y * mapScale * 100.0f;
    drawList->AddCircleFilled(ImVec2(rx, ry), 2.0f, IM_COL32(255, 0, 0, 255));

    ImGui::End();
}