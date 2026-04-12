#pragma once

#include "World/World.hpp"
#include "pch.hpp"

// CollisionSystem — 靜態 AABB 碰撞解決系統（Step 3.2 起改為 World 多房間版本）
//
// 使用方式：
//   1. App::Start()      呼叫 SetWorld() 設定當前世界
//   2. Player::Update()  呼叫 ResolveWall() 解決玩家 ↔ 牆壁碰撞
//   3. TryMove()         呼叫 IsBlocked() 供敵人 AI 避障
class CollisionSystem {
public:
    // 設定碰撞所用的世界（每次生成新層時重新呼叫）
    static void SetWorld(const World* world);

    // 解決實體 ↔ 牆壁碰撞，直接修改 worldPos
    static void ResolveWall(glm::vec2& worldPos,
                            glm::vec2  hitOffset,
                            glm::vec2  hitSize);

    // 敵人 AI 使用：檢查 AABB 是否被牆阻擋
    static bool IsBlocked(glm::vec2 center, glm::vec2 size);

private:
    static const World* s_World;
};
