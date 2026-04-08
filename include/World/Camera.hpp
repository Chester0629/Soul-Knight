#pragma once

#include "pch.hpp"

// Camera — 相機系統（即時跟隨玩家，無 Lerp）
// 使用方式：
//   每幀先呼叫 Camera::Update(playerWorldPos)
//   再將 Camera::GetPosition() 傳給所有物件的 SyncRenderTransform()
class Camera {
public:
    // 以玩家世界座標更新相機位置（M1：即時追蹤，無 Lerp）
    static void Update(glm::vec2 playerWorldPos) {
        m_Position = playerWorldPos;
    }

    // 取得當前相機世界座標（供所有物件計算渲染偏移）
    static glm::vec2 GetPosition() { return m_Position; }

private:
    static glm::vec2 m_Position;
};
