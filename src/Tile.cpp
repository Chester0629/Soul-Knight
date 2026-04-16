#include "Tile.hpp"

// ── Tile 基礎 ─────────────────────────────────────────────────────────────────
Tile::Tile(glm::vec2 worldPos) : m_WorldPos(worldPos) {
    m_Transform.scale = {3.0f, 3.0f};  // 16px → 48px
}

void Tile::SyncTransform(glm::vec2 cameraPos) {
    m_Transform.translation = m_WorldPos - cameraPos;
}

// ── FloorTile ─────────────────────────────────────────────────────────────────
FloorTile::FloorTile(glm::vec2 worldPos) : Tile(worldPos) {
    SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/Tiles/f101.png"));
    SetZIndex(0.0f);
    SetVisible(true);
}

// ── WallTile（頂蓋 / 底基 / 側牆）────────────────────────────────────────────
WallTile::WallTile(glm::vec2 worldPos) : Tile(worldPos) {
    m_IsWall = true;
    SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/Tiles/w001.png"));
    SetZIndex(0.5f);
    SetVisible(true);
}

// ── NorthFaceTile（北牆面，固定 Z）────────────────────────────────────────────
// w004 為 16×8px → scale(3,3) = 48×24px（格子高度的一半）。
// 中心往上移 TILE_SIZE/4（12px），使 Sprite 佔據格子上半部 [worldY, worldY+24]。
// 底部（worldY）恰好對齊 row2 地板上移後的頂部，視覺上無縫連接。
NorthFaceTile::NorthFaceTile(glm::vec2 worldPos) : Tile(worldPos) {
    m_IsWall = true;
    m_WorldPos.y += TILE_SIZE * 0.25f;  // +12px，sprite 佔格子上半部
    SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/Tiles/w004.png"));
    SetZIndex(0.3f);
    SetVisible(true);
}

// ── SouthFaceTile（南牆面，Y-Sort Z）─────────────────────────────────────────
// 同 NorthFaceTile，偏移 +3*TILE_SIZE/4 對齊新 w004 尺寸。
SouthFaceTile::SouthFaceTile(glm::vec2 worldPos) : Tile(worldPos) {
    m_IsWall = true;
    m_WorldPos.y += TILE_SIZE * 0.25f;  // 36px
    SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/Tiles/w004.png"));
    UpdateZIndex();
    SetVisible(true);
}

// ── SideWallFaceTile（東西側牆面，固定 Z）────────────────────────────────────
SideWallFaceTile::SideWallFaceTile(glm::vec2 worldPos) : Tile(worldPos) {
    m_IsWall = false;
    m_WorldPos.y += TILE_SIZE * 0.25f;
    SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/Tiles/w004.png"));
    SetZIndex(0.5f);  // 固定同 WallTile；門的 Y-Sort 主導渲染層
    SetVisible(true);
}

// ── DoorTile（門，可切換開關）────────────────────────────────────────────────
// ww001：16×32 → scale(3,3) = 48×96px（比一格高）
//   關閉時 m_WorldPos 上移 TILE_SIZE/2（24px），使 Sprite 底部 = 格子底部
// ww002：16×16 → scale(3,3) = 48×48px（正常格子大小）
//   開啟時還原 m_WorldPos 至原始格子座標
DoorTile::DoorTile(glm::vec2 worldPos)
    : Tile(worldPos), m_BaseWorldPos(worldPos)
{
    m_IsWall = true;
    SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/Tiles/ww001.png"));
    m_BaseZ = glm::clamp(50.0f - worldPos.y / 64.0f, 2.0f, 98.0f);
    SetZIndex(m_BaseZ);
    SetVisible(true);
}

void DoorTile::Open() {
    m_IsWall = false;
    m_WorldPos = m_BaseWorldPos;
    m_WorldPos.y -= TILE_SIZE / 2.0f;
    SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/Tiles/ww002.png"));
    SetZIndex(m_BaseZ);
}

void DoorTile::Close() {
    m_IsWall = true;
    m_WorldPos = m_BaseWorldPos;
    SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/Tiles/ww001.png"));
    SetZIndex(m_BaseZ);
}

void SouthFaceTile::UpdateZIndex() {
    // PTSD 的 nearClip=-100, farClip=100，zIndex 直接作為 3D Z 座標使用
    // ⚠️ Z 必須嚴格在 (-100, 100) 範圍內，否則被 GPU 裁剪不可見！
    // 公式：worldY 越小（越往南/畫面下方）→ Z 越大（越前面）
    // worldY ≈ +288（北側地板） → Z ≈ 2.0f（後面）
    // worldY ≈ -288（南側地板） → Z ≈ 98.0f（前面）
    // ⚠️ 上限必須 ≥ 玩家最大 Z(≈98)，否則玩家會畫在南牆面前方
    SetZIndex(glm::clamp(50.0f - m_WorldPos.y / 64.0f, 2.0f, 98.0f));
}
