#pragma once

#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "pch.hpp"

#include <memory>
#include <vector>

// MiniMap — 右上角迷你地圖
// 每個 Room 對應一個灰色方塊（SetVisible 控制已探索才顯示）
// 當前房間以黃色游標覆蓋
class MiniMap {
public:
    void Init(int roomCount, const std::vector<glm::ivec2>& gridPositions);
    void AddToRenderer(Util::Renderer& root);
    void Update(int currentRoomIdx, const std::vector<bool>& visited);

    static constexpr float Z_MAP = 99.45f;

private:
    // 5×5 地城網格映射到右上角螢幕區域
    // grid (2,2) = MAP 中心
    static constexpr float CELL_W  = 14.0f;
    static constexpr float CELL_H  = 11.0f;
    static constexpr float BLOCK_W = 12.0f;
    static constexpr float BLOCK_H =  9.0f;

    // 迷你地圖中心：右上角留 8px margin + 5格半幅偏移
    static constexpr float MAP_CX = 640.0f - 8.0f - (5.0f * CELL_W) / 2.0f;
    static constexpr float MAP_CY = 360.0f - 8.0f - (5.0f * CELL_H) / 2.0f;

    // 4×4 PNG 素材縮放比例
    static constexpr float SX = BLOCK_W / 4.0f;
    static constexpr float SY = BLOCK_H / 4.0f;

    static glm::vec2 GridToScreen(glm::ivec2 gridPos);

    std::vector<std::shared_ptr<Util::GameObject>> m_RoomObjs;
    std::shared_ptr<Util::GameObject>              m_Cursor;
    std::vector<glm::ivec2>                        m_GridPos;
};
