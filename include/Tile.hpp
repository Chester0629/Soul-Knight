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

// ── 東西牆面 Tile（東西門開口處的牆面，固定 Z）──────────────────────────────
// 位置：東西門 DoorTile 同一格，作為視覺覆蓋層
// 素材：w004.png，Z = 0.5f（固定，同 WallTile；門的 Y-Sort Z 主導渲染順序）
// 僅視覺用：m_IsWall = false，不影響碰撞
class SideWallFaceTile : public Tile {
public:
    explicit SideWallFaceTile(glm::vec2 worldPos);
};

// ── 門 Tile（走廊銜接處，可開關）────────────────────────────────────────────
// 關閉：ww001.png（16×32，scale(3,3)=48×96px），IsWall=true
//   → m_WorldPos 上移 TILE_SIZE/2 使 Sprite 底部對齊格子底部（同 NorthFaceTile 手法）
// 開啟：ww002.png（16×16，48×48px），IsWall=false
//   → m_WorldPos 還原為原始格子座標（無偏移）
class DoorTile : public Tile {
public:
    explicit DoorTile(glm::vec2 worldPos);
    void Open();
    void Close();
    bool IsOpen() const { return !m_IsWall; }
private:
    glm::vec2 m_BaseWorldPos;
    float     m_BaseZ;  // Open/Close 共用的 Y-Sort Z
};
