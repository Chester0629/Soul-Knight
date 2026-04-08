#pragma once

#include "Room.hpp"
#include "pch.hpp"

// CollisionSystem — 靜態 AABB 碰撞解決系統
//
// 使用方式：
//   1. App::Start()          呼叫 SetRoom() 設定當前房間
//   2. Player::Update()      呼叫 ResolveWall() 解決玩家 ↔ 牆壁碰撞
//   3. (M2+) TryMove()       呼叫 IsBlocked() 供敵人 AI 避障
//
// 碰撞原理（穿透深度最小軸 Push Back）：
//   計算 X / Y 各維度的重疊量，從「重疊最淺」的軸推回。
//   ⚠️ 禁止暴力歸位（worldPos = prevPos）。
class CollisionSystem {
public:
    // 設定碰撞所用的房間（每次房間切換時重新呼叫）
    static void SetRoom(const Room* room);

    // 解決實體 ↔ 牆壁碰撞，直接修改 worldPos
    //   hitOffset : 碰撞中心相對於 worldPos 的偏移（玩家 = {0, HIT_OFFSET_Y}）
    //   hitSize   : 碰撞盒大小（玩家 = {HIT_W, HIT_H}）
    static void ResolveWall(glm::vec2& worldPos,
                            glm::vec2  hitOffset,
                            glm::vec2  hitSize);

    // 敵人 AI 使用：檢查以 center 為中心、size 為大小的 AABB 是否被牆阻擋
    static bool IsBlocked(glm::vec2 center, glm::vec2 size);

private:
    static const Room* s_Room;

    // 對單一牆壁 Tile 執行穿透深度 Push Back（直接修改 entityCenter）
    static void ResolveAABB(glm::vec2& entityCenter,
                            glm::vec2  entityHalf,
                            glm::vec2  wallCenter,
                            float      wallHalf);
};
