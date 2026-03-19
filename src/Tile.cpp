#include "Tile.hpp"

Tile::Tile(TileType type, const std::string& imagePath, const glm::vec2& worldPos)
    : m_Type(type) {

    // 設定圖片素材
    SetDrawable(std::make_shared<Util::Image>(imagePath));

    // 設定世界座標
    m_Transform.translation = worldPos;

    // 設定 Z-index
    // 地板在最底層；牆壁略高（但仍低於角色的 y-sort 值 ~1000-worldY）
    if (type == TileType::FLOOR) {
        SetZIndex(-100.0f);
    } else {
        SetZIndex(0.0f);
    }

    SetVisible(true);
}
