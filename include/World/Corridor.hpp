#pragma once

#include "Tile.hpp"
#include "Util/Renderer.hpp"

#include <memory>
#include <vector>

// Corridor — 連接兩個 Room 的走廊
//
// 水平走廊（isHorizontal=true）：rows = CORR_W = 8 固定，cols 可變
//   row 0 / row 7 : WallTile（上下牆）
//   row 1..6      : FloorTile（通道）
//   左右兩端對接 Room 的門洞，無邊牆
//
// 垂直走廊（isHorizontal=false）：cols = CORR_W = 8 固定，rows 可變
//   col 0 / col 7 : WallTile（左右牆）
//   col 1..6      : FloorTile（通道）
class Corridor {
public:
    Corridor(glm::vec2 centerPos, int cols, int rows, bool isHorizontal);

    void AddToRenderer(Util::Renderer& renderer);
    void SyncTransforms(glm::vec2 cameraPos);

    // 碰撞介面（供 World 使用）
    glm::vec2 GetWorldOffset() const { return m_Center; }
    int       GetCols() const { return m_Cols; }
    int       GetRows() const { return m_Rows; }
    bool      IsWallAtGrid(int row, int col) const;

    // 座標換算（與 Room 相同公式）
    glm::vec2 TileToWorld(int row, int col) const;

private:
    glm::vec2 m_Center;
    int       m_Cols;
    int       m_Rows;
    bool      m_IsHorizontal;

    std::vector<std::vector<std::shared_ptr<Tile>>> m_TileMap;

    void Build();
};
