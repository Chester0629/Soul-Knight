#pragma once

#include "Room.hpp"
#include "World/Corridor.hpp"
#include "World/DungeonGenerator.hpp"
#include "Util/Renderer.hpp"

#include <memory>
#include <vector>

// World — 持有一層地城的所有 Room + Corridor，提供統一的碰撞介面
//
// 使用流程：
//   1. World::Generate(seed, floorIndex) — 生成佈局、建 Room/Corridor 物件
//   2. World::AddToRenderer(root)        — 將所有 Tile 加入渲染樹
//   3. 每幀 World::SyncTransforms(cameraPos) — 同步渲染位置
//   4. CollisionSystem::SetWorld(world)  — 設定碰撞對象
class World {
public:
    void Generate(unsigned seed, int floorIndex = 1);
    void AddToRenderer(Util::Renderer& renderer);
    void SyncTransforms(glm::vec2 cameraPos);

    // 碰撞介面（供 CollisionSystem 代理）
    void ResolveWall(glm::vec2& worldPos, glm::vec2 hitOffset, glm::vec2 hitSize) const;
    bool IsBlocked(glm::vec2 center, glm::vec2 size) const;

    // 玩家出生位置 = 初始房間（Spawn）世界座標
    glm::vec2 GetSpawnPos() const;

    // 5×5 網格的世界間距（= 36 tiles × 48px）
    static constexpr float GRID_SPACING_X = 1728.0f;
    static constexpr float GRID_SPACING_Y = 1296.0f;

private:
    std::vector<std::unique_ptr<Room>>     m_Rooms;
    std::vector<std::unique_ptr<Corridor>> m_Corridors;

    // (gridRow, gridCol) → 世界中心座標
    static glm::vec2 GridToWorld(glm::ivec2 gridPos);

    // 對單一 Tilemap 做 AABB Push Back（ResolveWall / IsBlocked 共用）
    static void ResolveAgainstTilemap(
        glm::vec2&       hitCenter,
        glm::vec2        hitHalf,
        glm::vec2        worldOffset,
        int              cols,
        int              rows,
        bool (*isWallFn)(int, int, const void*),
        const void*      ctx);

    static bool IsBlockedByTilemap(
        glm::vec2        center,
        glm::vec2        halfSize,
        glm::vec2        worldOffset,
        int              cols,
        int              rows,
        bool (*isWallFn)(int, int, const void*),
        const void*      ctx);
};
