#pragma once

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

#include <string>

/**
 * @brief 地圖格 (Tile) 的種類
 */
enum class TileType {
    FLOOR,      // 地板（可行走）
    WALL_TOP,   // 頂部牆（有上方圖案）
    WALL_SIDE,  // 側邊牆
    NONE        // 空格（走廊用）
};

/**
 * @brief 地圖格基礎類別
 *
 * 每個 Tile 繼承自 Util::GameObject，負責渲染一個 32x32 的靜態圖片。
 * Z-index 規則：
 *   - 地板 (FLOOR)    = -100（最底層）
 *   - 牆壁 (WALL_*)   = 0（比地板高，比角色低）
 */
class Tile : public Util::GameObject {
public:
    // 每個 Tile 在螢幕上的像素大小（縮放後）
    static constexpr float TILE_SIZE = 32.0f;

    /**
     * @param type     Tile 種類
     * @param imagePath 素材路徑（相對或絕對）
     * @param worldPos  世界座標 (pixel)，使用 PTSD 座標系（Y 向上，原點在螢幕中心）
     */
    Tile(TileType type, const std::string& imagePath, const glm::vec2& worldPos);

    TileType GetType() const { return m_Type; }

private:
    TileType m_Type;
};
