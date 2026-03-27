#include "System/CollisionSystem.hpp"

#include <cmath>

const Room* CollisionSystem::s_Room = nullptr;

void CollisionSystem::SetRoom(const Room* room) {
    s_Room = room;
}

// ── 穿透深度最小軸 Push Back ─────────────────────────────────────────────────
// overlapX / overlapY：各軸的 AABB 重疊量
// 從重疊最淺的軸推回，讓另一軸保持滑動（沿牆行走）
void CollisionSystem::ResolveAABB(glm::vec2& entityCenter,
                                   glm::vec2  entityHalf,
                                   glm::vec2  wallCenter,
                                   float      wallHalf) {
    const float overlapX = entityHalf.x + wallHalf
                         - std::abs(entityCenter.x - wallCenter.x);
    const float overlapY = entityHalf.y + wallHalf
                         - std::abs(entityCenter.y - wallCenter.y);

    if (overlapX <= 0.0f || overlapY <= 0.0f) return;  // 無重疊

    if (overlapX < overlapY)
        entityCenter.x += (entityCenter.x < wallCenter.x) ? -overlapX : overlapX;
    else
        entityCenter.y += (entityCenter.y < wallCenter.y) ? -overlapY : overlapY;
}

// ── 世界座標 → Tile 索引（TileToWorld 的反函式）────────────────────────────
// TileToWorld 定義：
//   worldX =  col * TILE_SIZE - mapCols * TILE_SIZE / 2 + TILE_SIZE / 2
//   worldY = -(row * TILE_SIZE - mapRows * TILE_SIZE / 2 + TILE_SIZE / 2)
// 反推：
//   col = worldX / TILE_SIZE + (mapCols - 1) / 2.0f
//   row = -worldY / TILE_SIZE + (mapRows - 1) / 2.0f
static int worldToCol(float wx, int cols) {
    return static_cast<int>(std::floor(wx / TILE_SIZE + (cols - 1) * 0.5f));
}
static int worldToRow(float wy, int rows) {
    return static_cast<int>(std::floor(-wy / TILE_SIZE + (rows - 1) * 0.5f));
}

// ── 解決實體 ↔ 牆壁碰撞 ──────────────────────────────────────────────────────
void CollisionSystem::ResolveWall(glm::vec2& worldPos,
                                   glm::vec2  hitOffset,
                                   glm::vec2  hitSize) {
    if (!s_Room) return;

    const int   rows     = s_Room->GetRows();
    const int   cols     = s_Room->GetCols();
    const float wallHalf = TILE_SIZE / 2.0f;  // 24.0f
    const glm::vec2 hitHalf = hitSize * 0.5f;

    glm::vec2 hitCenter = worldPos + hitOffset;

    // 求可能相交的 Tile 索引範圍，±1 格保護邊距防止遺漏邊界格
    // ⚠️ worldToRow(高 Y) < worldToRow(低 Y)，因為 row 向下遞增
    const int rMin = std::max(worldToRow(hitCenter.y + hitHalf.y, rows) - 1, 0);
    const int rMax = std::min(worldToRow(hitCenter.y - hitHalf.y, rows) + 1, rows - 1);
    const int cMin = std::max(worldToCol(hitCenter.x - hitHalf.x, cols) - 1, 0);
    const int cMax = std::min(worldToCol(hitCenter.x + hitHalf.x, cols) + 1, cols - 1);

    for (int r = rMin; r <= rMax; ++r) {
        for (int c = cMin; c <= cMax; ++c) {
            if (!s_Room->IsWallAt(r, c)) continue;
            const glm::vec2 wallCenter = Room::TileToWorld(r, c, rows, cols);
            ResolveAABB(hitCenter, hitHalf, wallCenter, wallHalf);
        }
    }

    worldPos = hitCenter - hitOffset;
}

// ── IsBlocked（敵人 AI TryMove 避障）────────────────────────────────────────
bool CollisionSystem::IsBlocked(glm::vec2 center, glm::vec2 size) {
    if (!s_Room) return false;

    const int   rows     = s_Room->GetRows();
    const int   cols     = s_Room->GetCols();
    const float wallHalf = TILE_SIZE / 2.0f;
    const glm::vec2 halfSize = size * 0.5f;

    const int rMin = std::max(worldToRow(center.y + halfSize.y, rows) - 1, 0);
    const int rMax = std::min(worldToRow(center.y - halfSize.y, rows) + 1, rows - 1);
    const int cMin = std::max(worldToCol(center.x - halfSize.x, cols) - 1, 0);
    const int cMax = std::min(worldToCol(center.x + halfSize.x, cols) + 1, cols - 1);

    for (int r = rMin; r <= rMax; ++r) {
        for (int c = cMin; c <= cMax; ++c) {
            if (!s_Room->IsWallAt(r, c)) continue;
            const glm::vec2 wallCenter = Room::TileToWorld(r, c, rows, cols);
            const float ox = halfSize.x + wallHalf - std::abs(center.x - wallCenter.x);
            const float oy = halfSize.y + wallHalf - std::abs(center.y - wallCenter.y);
            if (ox > 0.0f && oy > 0.0f) return true;
        }
    }
    return false;
}
