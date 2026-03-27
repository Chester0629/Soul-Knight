#pragma once

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

// 縮放後每格像素大小：16px 原圖 × scale(3,3) = 48px
constexpr float TILE_SIZE = 48.0f;

// ── Tile 基礎類別 ────────────────────────────────────────────────────────────
// 核心規則：
//   m_WorldPos            = 物理/碰撞的唯一真實座標
//   m_Transform.translation = 僅供 PTSD Renderer 渲染，由 SyncTransform() 更新
class Tile : public Util::GameObject {
public:
    explicit Tile(glm::vec2 worldPos);
    virtual ~Tile() = default;

    bool      IsWall()      const { return m_IsWall; }
    glm::vec2 GetWorldPos() const { return m_WorldPos; }

    // 每幀同步渲染位置；Step 1.4 前傳 {0,0}
    void SyncTransform(glm::vec2 cameraPos);

protected:
    glm::vec2 m_WorldPos;
    bool      m_IsWall = false;
};

// ── 地板 Tile ─────────────────────────────────────────────────────────────────
// 素材：f101.png，Z = 0.0f
class FloorTile : public Tile {
public:
    explicit FloorTile(glm::vec2 worldPos);
};

// ── 牆頂/牆基 Tile（頂蓋 + 底基 + 左右側牆）─────────────────────────────────
// 素材：w001.png，Z = 0.5f（固定，略高於地板，遠低於角色）
class WallTile : public Tile {
public:
    explicit WallTile(glm::vec2 worldPos);
};

// ── 北牆面 Tile（北牆內側可見的磚面）────────────────────────────────────────
// 位置：緊靠北牆內側（row 1）
// 素材：w004.png，Z = 0.3f（低於 WallTile 0.5f，讓 row 0 cap 蓋住重疊區）
// 邏輯：玩家在其南側行走，應渲染於玩家之後 → 低固定 Z
class NorthFaceTile : public Tile {
public:
    explicit NorthFaceTile(glm::vec2 worldPos);
};

// ── 南牆面 Tile（南牆內側可見的磚面）────────────────────────────────────────
// 位置：緊靠南牆內側（row ROWS-2）
// 素材：w004.png，Z = Y-Sort（玩家走近時遮擋效果正確）
class SouthFaceTile : public Tile {
public:
    explicit SouthFaceTile(glm::vec2 worldPos);
    // 每幀依 worldY 更新 Z-index（Y-Sort）
    void UpdateZIndex();
};
