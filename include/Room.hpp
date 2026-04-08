#pragma once

#include "Tile.hpp"
#include "Util/Renderer.hpp"

#include <memory>
#include <vector>

// ── 房間尺寸規格（總格數，含所有牆壁行）───────────────────────────────────────
// 初始房間：17 cols × 17 rows（含牆）
//   牆壁結構（北/南各 2 行，左/右各 1 列）：
//     Row 0          : WallTile   cap   (w001，通常超出螢幕)
//     Row 1          : NorthFace  (w004，可見，固定 Z=0.6f)
//     Row 2 ~ ROWS-3 : 地板 + 側牆
//     Row ROWS-2     : SouthFace  (w004，可見，Y-Sort Z)
//     Row ROWS-1     : WallTile   base  (w001，通常超出螢幕)
//
// 怪物房間：22~32 cols × 22~32 rows（含牆）
// 走廊：    8 cols 或 8 rows（6 地板 + 兩側各 1 牆）
namespace RoomSpec {
    constexpr int START_W = 17;   // 初始房間寬（含牆）
    constexpr int START_H = 17;   // 初始房間高（含牆）
    constexpr int MON_MIN = 22;   // 怪物房間最小邊長（含牆）
    constexpr int MON_MAX = 32;   // 怪物房間最大邊長（含牆）
    constexpr int CORR_W  = 8;    // 走廊總寬（6 地板 + 兩壁）
}

// ── Room ──────────────────────────────────────────────────────────────────────
class Room {
public:
    explicit Room(int cols = RoomSpec::START_W, int rows = RoomSpec::START_H);
    ~Room() = default;

    void AddToRenderer(Util::Renderer& renderer);
    void SyncTransforms(glm::vec2 cameraPos);

    bool IsWallAt(int row, int col) const;
    int GetCols() const { return m_Cols; }
    int GetRows() const { return m_Rows; }

    static glm::vec2 TileToWorld(int row, int col, int mapRows, int mapCols);

private:
    void Build();

    int m_Cols;
    int m_Rows;

    std::vector<std::vector<std::shared_ptr<Tile>>> m_TileMap;

    // Y-Sort 更新清單（南牆面）
    std::vector<std::shared_ptr<SouthFaceTile>> m_SouthFaces;

    // NorthFaceTile 上移後，原 row 1 格子空出，用 FloorTile 回填
    std::vector<std::shared_ptr<Tile>> m_BackfillTiles;
};
