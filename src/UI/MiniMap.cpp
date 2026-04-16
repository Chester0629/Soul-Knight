#include "UI/MiniMap.hpp"
#include "Util/Image.hpp"

glm::vec2 MiniMap::GridToScreen(glm::ivec2 gridPos) {
    return {
        MAP_CX + (gridPos.y - 2) * CELL_W,
        MAP_CY - (gridPos.x - 2) * CELL_H
    };
}

void MiniMap::Init(int roomCount, const std::vector<glm::ivec2>& gridPositions) {
    m_GridPos = gridPositions;
    m_RoomObjs.clear();

    const std::string roomImg = RESOURCE_DIR "/UI/minimap_room.png";

    for (int i = 0; i < roomCount; ++i) {
        auto obj = std::make_shared<Util::GameObject>();
        obj->SetDrawable(std::make_shared<Util::Image>(roomImg));
        obj->m_Transform.scale       = {SX, SY};
        obj->m_Transform.translation = GridToScreen(gridPositions[i]);
        obj->SetZIndex(Z_MAP);
        obj->SetVisible(false);
        m_RoomObjs.push_back(obj);
    }

    m_Cursor = std::make_shared<Util::GameObject>();
    m_Cursor->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/UI/minimap_cur.png"));
    m_Cursor->m_Transform.scale = {SX, SY};
    m_Cursor->SetZIndex(Z_MAP + 0.01f);
    m_Cursor->SetVisible(false);
}

void MiniMap::AddToRenderer(Util::Renderer& root) {
    for (auto& obj : m_RoomObjs)
        root.AddChild(obj);
    root.AddChild(m_Cursor);
}

void MiniMap::Update(int currentRoomIdx, const std::vector<bool>& visited) {
    for (int i = 0; i < static_cast<int>(m_RoomObjs.size()); ++i)
        m_RoomObjs[i]->SetVisible(i < static_cast<int>(visited.size()) && visited[i]);

    if (currentRoomIdx >= 0 && currentRoomIdx < static_cast<int>(m_GridPos.size())) {
        m_Cursor->m_Transform.translation = GridToScreen(m_GridPos[currentRoomIdx]);
        m_Cursor->SetVisible(true);
    } else {
        m_Cursor->SetVisible(false);
    }
}
