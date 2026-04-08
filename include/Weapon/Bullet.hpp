#pragma once

#include "Util/GameObject.hpp"
#include "pch.hpp"

// Bullet — 子彈實體，由 BulletManager 對象池管理
// 規格：
//   Z-Index 固定 99.0f（永遠在玩家/敵人之上，且在 PTSD farClip=100 內）
//   Hitbox 統一使用 10×10px 核心小判定盒（與圖片尺寸無關）
//   m_Lifetime=-1 → 不限時；>0 → 秒數倒數後自動 Deactivate
class Bullet : public Util::GameObject {
public:
    Bullet();

    static constexpr float HIT_SIZE = 10.0f;  // 10×10 判定盒
    static constexpr float Z_INDEX  = 99.0f;

    glm::vec2 m_WorldPos  = {0.0f, 0.0f};
    glm::vec2 m_Velocity  = {0.0f, 0.0f};
    float     m_Lifetime  = -1.0f;   // -1=不限時；>0=秒數倒數
    int       m_Damage    = 0;
    bool      m_Active    = false;
    bool      m_IsPlayer  = false;   // true=玩家子彈，false=敵人子彈
};
