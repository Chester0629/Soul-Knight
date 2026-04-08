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
// 使用 w004.png：北牆內側可見面（Row 1），玩家看到的是牆的正面。
// SouthFaceTile 同樣使用 w004（南牆立體深度效果）。
NorthFaceTile::NorthFaceTile(glm::vec2 worldPos) : Tile(worldPos) {
    m_IsWall = true;
    SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/Tiles/w004.png"));
    SetZIndex(0.3f);  // 低於 WallTile(0.5)，讓 row 0 cap 蓋住 NorthFace 的重疊區
    SetVisible(true);
}

// ── SouthFaceTile（南牆面，Y-Sort Z）─────────────────────────────────────────
// 玩家走近南牆時，南牆面應遮擋玩家 → Y-Sort
SouthFaceTile::SouthFaceTile(glm::vec2 worldPos) : Tile(worldPos) {
    m_IsWall = true;
    SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/Tiles/w004.png"));
    UpdateZIndex();  // 設定初始 Z
    SetVisible(true);
}

void SouthFaceTile::UpdateZIndex() {
    // PTSD 的 nearClip=-100, farClip=100，zIndex 直接作為 3D Z 座標使用
    // ⚠️ Z 必須嚴格在 (-100, 100) 範圍內，否則被 GPU 裁剪不可見！
    // 公式：worldY 越小（越往南/畫面下方）→ Z 越大（越前面）
    // worldY ≈ +288（北側地板） → Z ≈ 2.0f（後面）
    // worldY ≈ -288（南側地板） → Z ≈ 98.0f（前面）
    // ⚠️ 上限必須 ≥ 玩家最大 Z(≈98)，否則玩家會畫在南牆面前方
    SetZIndex(glm::clamp(50.0f - m_WorldPos.y / 6.0f, 2.0f, 98.0f));
}
