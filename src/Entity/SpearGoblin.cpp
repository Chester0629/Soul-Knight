#include "Entity/Enemy.hpp"
#include "Entity/Player.hpp"

// SpearGoblin AI：
//   dist > m_StabRange → 直線追蹤玩家（TryMove）
//   dist ≤ m_StabRange 且冷卻完成 → 突刺攻擊（Step 2.3 前為空操作）
void SpearGoblin::UpdateAI(float dt) {
    if (!m_Target) return;

    const glm::vec2 toPlayer = m_Target->GetWorldPos() - m_WorldPos;
    const float     dist     = glm::length(toPlayer);

    // ── 追蹤 ──────────────────────────────────────────────────────────────────
    if (dist > m_StabRange && dist > 0.01f) {
        TryMove(glm::normalize(toPlayer), m_Speed, dt);
    }

    // 面向玩家
    if (dist > 0.01f)
        m_Transform.scale.x = (toPlayer.x < 0.0f) ? -3.0f : 3.0f;

    // ── 突刺攻擊（Step 2.3 前為空操作） ──────────────────────────────────────
    if (dist <= m_StabRange && m_AttackCooldown <= 0.0f) {
        // TODO (Step 2.3): 展開突刺碰撞盒，對 hitbox 內的玩家造成傷害
        m_AttackCooldown = COOLDOWN;
    }
}
