#pragma once

#include "Tile.hpp"
#include "Util/Renderer.hpp"

#include <memory>
#include <random>
#include <string>
#include <vector>

// Corridor — 連接兩個 Room 的走廊
//
// 水平走廊（isHorizontal=true）：rows = H_CORR_W = 9 固定，cols 可變
//   row 0           : WallTile cap（頂蓋）
//   row 1           : NorthFaceTile（+TILE_SIZE Y 偏移，與 Room 相同手法）
//   row 2..6        : FloorTile（5 格通道）
//   row 7           : SouthWallCap（Z=97.5f）
//   row 8           : SouthFaceTile（+TILE_SIZE Y 偏移，Y-Sort Z）
//
// 垂直走廊（isHorizontal=false）：cols = V_CORR_W = 8 固定，rows 可變
//   col 0 / col 7   : WallTile（左右牆）
//   col 1..6        : FloorTile（通道）
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
    std::vector<std::shared_ptr<SouthFaceTile>>     m_SouthFaces;    // Y-Sort 更新
    std::vector<std::shared_ptr<Tile>>              m_BottomFill;    // 補底地板（最後一排原始位置）

    void Build();

    std::mt19937 m_Rng;
    std::string  RandFloor();
    void ApplyWall(Util::GameObject* o, int s);
    void ApplyFace(Util::GameObject* o, int s);
};
